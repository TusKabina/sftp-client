#include "TransferManager.h"
#include <algorithm>
#include <QFileInfo>
#include <qstring.h>
#include "Utilities/Logger.h"

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
    QObject::connect(job, SIGNAL(onErrorMessage(std::string)),
        this, SLOT(onErrorMessageReceived(std::string)));
    return m_transferJobs.back()->getJobId();
}

void TransferManager::connect(const std::string& host, const std::string& username, std::string& password) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    if (!m_DirectoryCache.initialize(host, username, password)) {
        m_initialized = false;
        logger().critical() << "Failed to connect to the server";
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

void TransferManager::onErrorMessageReceived(const std::string errorMessage) {
    emit errorMessageSent(errorMessage);
}

void TransferManager::executeJob(const uint64_t jobId, JobOperation jobType, std::shared_ptr<CURL> curl) {
   const auto job = std::find_if(m_transferJobs.begin(), m_transferJobs.end(),[&jobId](const TransferJob* transferJob) {
        return transferJob->getJobId() == jobId;
    });
   
   if (job == m_transferJobs.end()) {
       std::cout << "JOB NOT FOUND" << std::endl;
       return;
   }
    std::string localDirPath = (*job)->getLocalDirectoryPath() + "/";
    (*job)->setTransferHandle(curl);

    if(job != m_transferJobs.end()) {
        std::string remoteDirPath = (*job)->getRemoteDirectoryPath() + "/";

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
            {
                QString localPath = QString::fromStdString((*job)->getLocalPath());

                QFileInfo localFile(localPath);
                uint64_t totalBytes = localFile.size();

                (*job)->setFileTotalBytes(totalBytes);
                (*job)->uploadFile(m_url);

                {
                    QMutexLocker locker(&m_mutex);
                    m_DirectoryCache.refreshDirectory(remoteDirPath);
                }
                break;
            }
            case JobOperation::COPY:
                {
                    QMutexLocker locker(&m_mutex);
                    std::string source = (*job)->getLocalDirectoryPath() + "/";
                    std::string sourceFileName = (*job)->getLocalPath().substr(source.size(), (*job)->getRemotePath().size());
                    uint64_t totalBytes = m_DirectoryCache.getTotalBytes(source, sourceFileName);
                    (*job)->setFileTotalBytes(totalBytes);
                }
                (*job)->copyFile();
                {
                    QMutexLocker locker(&m_mutex);
                    m_DirectoryCache.refreshDirectory(remoteDirPath);
                }
                break;
            case JobOperation::MOVE:
                (*job)->moveFile(m_url);
                {
                   QMutexLocker locker(&m_mutex);
                   m_DirectoryCache.refreshDirectory(localDirPath);
                   m_DirectoryCache.refreshDirectory(remoteDirPath);
                }
                break;
            case JobOperation::DELETE:
                (*job)->deleteFile(m_url);
                {
                    QMutexLocker locker(&m_mutex);
                    m_DirectoryCache.refreshDirectory(remoteDirPath);
                }
                break;
            case JobOperation::DELETE_LOCAL:
                (*job)->deleteLocalFile((*job)->getLocalPath());
                break;
            case JobOperation::MKDIR:
                (*job)->createDirectory((*job)->getRemotePath());
                {
                    QMutexLocker locker(&m_mutex);
                    m_DirectoryCache.refreshDirectory(remoteDirPath);
                }
                break;
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
        logger().critical() << "Job with job id: " << jobId << " Not found";
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

void TransferManager::reset() {
    m_DirectoryCache.reset();

    for (auto& handle : m_transferHandles) {
        handle.m_curlHandle.reset();
        handle.m_transferStatus.reset();
    }

    for (TransferJob* job : m_transferJobs) {
        delete job;
    }

    m_transferJobs.clear();
    m_username = "";
    m_password = "";
    m_url = "";
    m_maxHandlesNumber = 0;
    m_handleCounter = 0;
    m_initialized = false;

   // m_threadPool.cleanup();
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