#include "TransferManager.h"
#include <algorithm>

void TransferManager::setCredentials(const std::string& host, const std::string& username, const std::string& password) {
    m_url = "sftp://" + username + ":" + password + "@" + host;
    m_username = username;
    m_password = password;
}

uint64_t TransferManager::prepareJob(const std::string localPath, const std::string remotePath) {
    QMutexLocker locker(&m_mutex);
    TransferJob* job = new TransferJob(localPath, remotePath, m_url);
    m_transferJobs.push_back(job);

    QObject::connect(job, SIGNAL(onTransferStatusUpdated(TransferStatus)),
        this, SLOT(onTransferStatusReceived(TransferStatus)));

    return m_transferJobs.back()->getJobId();
}

void TransferManager::connect(const std::string& host, const std::string& username, std::string& password) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    if (!m_DirectoryCache.initialize(host, username, password)) {
        m_initialized = false;
        return;
    }
    setCredentials(host, username, password);

    m_DirectoryCache.prefetchDirectories("/", 3);
    m_initialized = true;
}

TransferManager::~TransferManager() {
    m_threadPool.waitForDone();
    curl_global_cleanup();
}

void TransferManager::executeJob(const uint64_t jobId, JobOperation jobType, std::shared_ptr<CURL> curl) {
   const auto job = std::find_if(m_transferJobs.begin(), m_transferJobs.end(),[&jobId](const TransferJob* transferJob) {
        return transferJob->getJobId() == jobId;
    });
   
   if (job == m_transferJobs.end()) {
       std::cout << "JOB NOT FOUND" << std::endl;
       return;
   }
    std::string localDirPath = (*job)->getLocalDirectoryPath();
    (*job)->setTransferHandle(curl);

    if(job != m_transferJobs.end()) {
        std::string remotePath = (*job)->getRemoteDirectoryPath() + "/";
        switch (jobType) {
            case JobOperation::DOWNLOAD:
                {
                    QMutexLocker locker(&m_mutex);
                    std::string remotePathDirectory = (*job)->getRemoteDirectoryPath() + "/";
                    std::string remoteFileName = (*job)->getRemotePath().substr(remotePathDirectory.size(), (*job)->getRemotePath().size());
                    uint64_t totalBytes = m_DirectoryCache.getTotalBytes(remotePathDirectory, remoteFileName);
                    (*job)->setFileTotalBytes(totalBytes);  
                }
                (*job)->downloadFile();
               // m_DirectoryCache.refreshDirectory(job->getLocalDirectoryPath() + "/");
                break;
            case JobOperation::UPLOAD:
                (*job)->uploadFile(m_url);
                {
                    QMutexLocker locker(&m_mutex);
                    m_DirectoryCache.refreshDirectory(remotePath);
                }
                break;
            case JobOperation::COPY:
                (*job)->copyFile();
                break;
            case JobOperation::MOVE:
               // m_DirectoryCache.refreshDirectory((*job)->getLocalDirectoryPath() + "/");
               // m_DirectoryCache.refreshDirectory((*job)->getRemoteDirectoryPath() + "/");
                (*job)->moveFile(m_url);
                break;
            case JobOperation::DELETE:
                (*job)->deleteFile(m_url);
                {
                    QMutexLocker locker(&m_mutex);
                    m_DirectoryCache.refreshDirectory((*job)->getRemoteDirectoryPath() + "/");
                }
                break;
            case JobOperation::DELETE_LOCAL:
                (*job)->deleteLocalFile((*job)->getLocalPath());

        }
    }
}

void TransferManager::submitJob(uint64_t jobId, JobOperation jobType) {
    auto runnable = new JobRunnable(this, jobId, jobType, &m_threadPool);
    runnable->setAutoDelete(false);
    m_threadPool.start(runnable);
}

const TransferJob* TransferManager::getJob(uint64_t jobId) const {
    auto job = std::find_if(m_transferJobs.begin(), m_transferJobs.end(),[&jobId](const TransferJob* transferJob) {
        return transferJob->getJobId() == jobId;
    });
    if(job == m_transferJobs.end()) {
        std::string err = "Job with job id: " + std::to_string(jobId) + " Not found";
        throw std::runtime_error(err.c_str());
    }
    return *job;
}

TransferHandle &TransferManager::findFreeHandle() {
    auto handle = std::find_if(begin(m_transferHandles), end(m_transferHandles), [](const TransferHandle& handle) {
        return handle.getTransferStatus().m_state == TransferStatus::TransferState::Initialized;
        });
    if (handle != m_transferHandles.end()) {
        return *handle;
    }
}

const std::vector<DirectoryEntry> TransferManager::getDirectoryList(const std::string &path) {
    std::vector<DirectoryEntry> entries;
    if (!m_DirectoryCache.isPathInCache(path)) {
        m_DirectoryCache.prefetchDirectories(path, 3);
    }
    m_DirectoryCache.getCachedDirectory(path, entries);
    return entries;
}

void TransferManager::deleteJob(uint64_t jobId) {
    for (auto it = m_transferJobs.begin(); it != m_transferJobs.end(); ) {
        if ((*it)->getJobId() == jobId) {
            delete* it;  
            it = m_transferJobs.erase(it);  
            break;
        }
        else {
            ++it;
        }
    }
}

void TransferManager::onTransferStatusReceived(TransferStatus status) {
    emit transferStatusUpdated(status);
}