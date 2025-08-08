#include "TransferManager.h"
#include <algorithm>
#include <QFileInfo>
#include <qstring.h>
#include "Utilities/MeasureHelper.h"
#include "Utilities/Logger.h"
#include <filesystem>

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

uint64_t TransferManager::prepareJobToResume(const std::string localPath, const std::string remotePath, const uint64_t bytesTransferred, const uint64_t totalBytes, 
                                                const uint64_t jobId, const TransferStatus::TransferOperation operation) {
    QMutexLocker locker(&m_mutex);
    TransferJob* job = new TransferJob(localPath, remotePath, m_url);

	job->setFileBytesTransferred(bytesTransferred);
	job->setJobId(jobId);
	job->setJobOperation(operation);
	job->setTotalBytes(totalBytes);

    m_transferJobs.push_back(job);

    QObject::connect(job, SIGNAL(onTransferStatusUpdated(TransferStatus)),
        this, SLOT(onTransferStatusReceived(TransferStatus)));
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

void TransferManager::executeJob(const uint64_t jobId, JobOperation jobType, std::shared_ptr<CURL> curl) {
   const auto job = std::find_if(m_transferJobs.begin(), m_transferJobs.end(),[&jobId](const TransferJob* transferJob) {
        return transferJob->getJobId() == jobId;
   });
   
   if (job == m_transferJobs.end()) {
       logger().critical() << "Job with job ID: " << jobId << "Not found!";
       return;
   }

   (*job)->setTransferHandle(curl);
   
   std::string localDirPath = (*job)->getLocalDirectoryPath() + "/";
   std::string remoteDirPath = (*job)->getRemoteDirectoryPath() + "/";

   switch (jobType) {
       case JobOperation::DOWNLOAD:
           downloadJob(*job);
           break;
       case JobOperation::UPLOAD:
           uploadJob(*job, remoteDirPath);
           break;
       case JobOperation::COPY:
           copyJob(*job, remoteDirPath);
           break;
       case JobOperation::MOVE:
           moveJob(*job, localDirPath, remoteDirPath);
           break;
       case JobOperation::DELETE:
           deleteJob(*job, remoteDirPath);
           break;
       case JobOperation::DELETE_LOCAL:
           deleteLocalJob(*job);
           break;
       case JobOperation::MKDIR:
           mkdirJob(*job, remoteDirPath);
           break;
	   case JobOperation::RESUME:
		   resumeJob(*job);
           break;
    }
}

void TransferManager::submitJob(uint64_t jobId, JobOperation jobType) {
    auto runnable = new JobRunnable(this, jobId, jobType, &m_threadPool);
    m_threadPool.start(runnable);
}

void TransferManager::cancelJob(uint64_t jobId) {
    auto job = std::find_if(m_transferJobs.begin(), m_transferJobs.end(),[&jobId](const TransferJob* transferJob) {
        return transferJob->getJobId() == jobId;
    });
    
    if (job == m_transferJobs.end()) {
        logger().critical() << "Job with job ID: " << jobId << " Not found!";
        return;
    }
	(*job)->cancelJob();
}

void TransferManager::pauseJob(uint64_t jobId) {
    auto job = std::find_if(m_transferJobs.begin(), m_transferJobs.end(),[&jobId](const TransferJob* transferJob) {
        return transferJob->getJobId() == jobId;
    });
    
    if (job == m_transferJobs.end()) {
        logger().critical() << "Job with job ID: " << jobId << " Not found!";
        return;
    }

    (*job)->pauseJob();
}

void TransferManager::resumeJob(const std::string& localPath, const std::string& remotePath, const uint64_t bytesTransfered, const uint64_t totalBytes, const uint64_t jobId,
                                const TransferStatus::TransferOperation operation) {

	// If operation is Download, check if the local file exists
    if (operation == TransferStatus::TransferOperation::Download) {
        if (std::filesystem::exists(localPath)) {
            logger().debug() << "Resuming job for file: " << localPath;
        } 
        else {
            logger().error() << "File does not exist: " << localPath;
            return;
        }
    }
    // If operation is Upload, check if the remote file exists
    else if (operation == TransferStatus::TransferOperation::Upload) {
        if (m_DirectoryCache.isFile(remotePath)) {
            logger().debug() << "Resuming job for file: " << remotePath;
        } 
        else {
            logger().error() << "Remote file does not exist: " << remotePath;
            return;
        }
	}

    uint64_t resumeJobId = prepareJobToResume(localPath, remotePath, bytesTransfered, totalBytes, jobId, operation);
    submitJob(resumeJobId, JobOperation::RESUME);
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

void TransferManager::downloadJob(TransferJob* job) {
    {
        QMutexLocker locker(&m_mutex);
        uint64_t totalBytes = m_DirectoryCache.getTotalBytes(job->getRemotePath());
        job->setFileTotalBytes(totalBytes);
		job->setJobOperation(TransferStatus::TransferOperation::Download);
    }

    job->downloadFile();

}

void TransferManager::resumeJob(TransferJob* job)
{
    switch (job->getJobOperation())
    {
        case TransferStatus::TransferOperation::Download:
            job->resumeDownloadFile();
            break;
        case TransferStatus::TransferOperation::Upload:
            job->resumeUploadFile();
            {
				QMutexLocker locker(&m_mutex);
                std::string source = job->getRemoteDirectoryPath() + "/";
                m_DirectoryCache.refreshDirectory(source);
            }
            break;

        default:
            break;
    }
}

void TransferManager::uploadJob(TransferJob* job, const std::string& source) {
    QString localPath = QString::fromStdString(job->getLocalPath());

    QFileInfo localFile(localPath);
    uint64_t totalBytes = localFile.size();

    job->setFileTotalBytes(totalBytes);
    job->setJobOperation(TransferStatus::TransferOperation::Upload);
    job->uploadFile();

    {
        QMutexLocker locker(&m_mutex);
        m_DirectoryCache.refreshDirectory(source);
    }
}

void TransferManager::copyJob(TransferJob* job, const std::string& source) {
    {
        QMutexLocker locker(&m_mutex);
        uint64_t totalBytes = m_DirectoryCache.getTotalBytes(job->getLocalPath());
        job->setFileTotalBytes(totalBytes);
        job->setJobOperation(TransferStatus::TransferOperation::Copy);
    }
    
    job->copyFile();
    
    {
        QMutexLocker locker(&m_mutex);
        m_DirectoryCache.refreshDirectory(source);
    }
}

void TransferManager::moveJob(TransferJob* job, const std::string& source, const std::string& destination) {
    job->moveFile();
    
    {
        QMutexLocker locker(&m_mutex);
        m_DirectoryCache.refreshDirectory(source);
        m_DirectoryCache.refreshDirectory(destination);
    }
}

void TransferManager::deleteJob(TransferJob* job, const std::string& source) {
    job->deleteFile();
   
    {
        QMutexLocker locker(&m_mutex);
        m_DirectoryCache.refreshDirectory(source);
    }
}

void TransferManager::deleteLocalJob(TransferJob* job) {
    job->deleteLocalFile(job->getLocalPath());
}

void TransferManager::mkdirJob(TransferJob* job, const std::string& source) {
    job->createDirectory(job->getRemotePath());
    
    {
        QMutexLocker locker(&m_mutex);
        m_DirectoryCache.refreshDirectory(source);
    }
}

void TransferManager::reset() {
    m_DirectoryCache.reset();

    for (TransferJob* job : m_transferJobs) {
        delete job;
    }

    m_transferJobs.clear();
    m_username = "";
    m_password = "";
    m_url = "";
    m_initialized = false;

}

const std::vector<DirectoryEntry> TransferManager::getDirectoryList(const std::string &path) {
    std::vector<DirectoryEntry> entries;
    if (!m_DirectoryCache.isPathInCache(path)) {
		logger().debug() << "Path not in cache, prefetching directories for path: " << path;
        m_DirectoryCache.prefetchDirectories(path, 3);
    }
    m_DirectoryCache.getCachedDirectory(path, entries);

	auto end = std::chrono::high_resolution_clock::now();
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

    logger().debug() << "TransferJobs size: " << m_transferJobs.size();
}

void TransferManager::onTransferStatusReceived(TransferStatus status) {
    emit transferStatusUpdated(status);
}