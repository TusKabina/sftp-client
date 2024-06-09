#include "TransferManager.h"
#include <algorithm>

void TransferManager::setCredentials(const std::string& host, const std::string& username, const std::string& password) {
    m_url = "sftp://" + username + ":" + password + "@" + host;
    m_username = username;
    m_password = password;
}

//TransferManager::TransferManager(const std::string& host, const std::string& username, std::string& password) {
//    //TODO: Number of transfer handles will depend on the number of threads that will be used. 10 is placeholder for now
//    curl_global_init(CURL_GLOBAL_DEFAULT);
//
//    if (!m_DirectoryCache.initialize(host, username, password)) {
//        m_initialized = false;
//        return;
//    }
//    setCredentials(host,username,password);
//    m_maxHandlesNumber = 10;
//    for(int id = 0; id < m_maxHandlesNumber; id++) {
//        std::shared_ptr<CURL> curlHandle(curl_easy_init(), curl_easy_cleanup);
//        m_transferHandles.emplace_back(std::move(curlHandle));
//    }
//
//    m_DirectoryCache.prefetchDirectories("/", 3);
//    m_initialized = true;
//}

uint64_t TransferManager::prepareJob(const std::string& localPath, const std::string& remotePath) {
    TransferJob job;
    job.createJob(m_transferHandles.at(m_handleCounter),localPath,remotePath);
    m_transferJobs.emplace_back(job);
    m_handleCounter++;
    return job.getJobId();
}

void TransferManager::connect(const std::string& host, const std::string& username, std::string& password) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    if (!m_DirectoryCache.initialize(host, username, password)) {
        m_initialized = false;
        return;
    }
    setCredentials(host, username, password);
    m_maxHandlesNumber = 10;
    for (int id = 0; id < m_maxHandlesNumber; id++) {
        std::shared_ptr<CURL> curlHandle(curl_easy_init(), curl_easy_cleanup);
        m_transferHandles.emplace_back(std::move(curlHandle));
    }

    m_DirectoryCache.prefetchDirectories("/", 3);
    m_initialized = true;
}

TransferManager::~TransferManager() {
    curl_global_cleanup();
}

void TransferManager::executeJob(const uint64_t jobId, JobOperation jobType) {
    const auto& job = std::find_if(m_transferJobs.begin(), m_transferJobs.end(),[&jobId](const TransferJob& transferJob) {
        return transferJob.getJobId() == jobId;
    });
    if(job != m_transferJobs.end()) {
        switch (jobType) {
            case JobOperation::DOWNLOAD:
                job->downloadFile(m_url);
                m_DirectoryCache.refreshDirectory(job->getLocalDirectoryPath() + "/");
                break;
            case JobOperation::UPLOAD:
                job->uploadFile(m_url);
                m_DirectoryCache.refreshDirectory(job->getRemoteDirectoryPath() + "/");
                break;
            case JobOperation::COPY:
                job->copyFile(m_url);
                break;
            case JobOperation::MOVE:
                job->moveFile(m_url);
                m_DirectoryCache.refreshDirectory(job->getLocalDirectoryPath() + "/");
                m_DirectoryCache.refreshDirectory(job->getRemoteDirectoryPath() + "/");
                break;
            case JobOperation::DELETE:
                job->deleteFile(m_url);
                m_DirectoryCache.refreshDirectory(job->getRemoteDirectoryPath() + "/");
                break;
        }
    }
}

const TransferJob &TransferManager::getJob(uint64_t jobId) const {
    const auto& job = std::find_if(m_transferJobs.begin(), m_transferJobs.end(),[&jobId](const TransferJob& transferJob) {
        return transferJob.getJobId() == jobId;
    });
    if(job == m_transferJobs.end()) {
        std::string err = "Job with job id: " + std::to_string(jobId) + " Not found";
        throw std::runtime_error(err.c_str());
    }
    return *job;
}

TransferHandle &TransferManager::findFreeHandle() {
    return *(std::find_if(begin(m_transferHandles),end(m_transferHandles),[](const TransferHandle& handle) {
        return handle.getTransferStatus().m_state == TransferStatus::TransferState::Initialized;
    }));
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
    m_transferJobs.erase(std::remove_if(
            m_transferJobs.begin(),
            m_transferJobs.end(),
            [jobId](const TransferJob& job){return job.getJobId() == jobId;}),end(m_transferJobs));
}
