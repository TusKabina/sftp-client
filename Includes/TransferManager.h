#ifndef SFTP_INTERFACE_TRANSFERMANAGER_H
#define SFTP_INTERFACE_TRANSFERMANAGER_H

#include <QMutex>
#include <QRunnable>
#include <QPointer>
#include <QThreadPool>
#include <QThreadStorage>
#include <vector>
#include <map>
#include <string>
#include "TransferJob.h"
#include "DirectoryCache.h"
#include "HandleDeleter.h"

class JobRunnable;

class CurlThreadPool : public QThreadPool {
public:
    CurlThreadPool() : QThreadPool() {
        unsigned int maxConcurrency = std::thread::hardware_concurrency();
        if (maxConcurrency < 1) {
            maxConcurrency = 20;
        }
        std::cout << "Initialized thread pool with: " << maxConcurrency << " number of threads\n";
        setMaxThreadCount(static_cast<int>(maxConcurrency));
    }

    std::shared_ptr<CURL> getCurlForCurrentThread() {
        if (!threadCurlStorage.hasLocalData()) {
            std::cout << "CURL INITIZLIED IN THREADPOOL" << std::endl;
            threadCurlStorage.setLocalData(std::shared_ptr<CURL>(curl_easy_init(), curl_easy_cleanup));
        }
        return threadCurlStorage.localData();
    }

    void cleanup() {
        threadCurlStorage.setLocalData(nullptr);
        clear();
        waitForDone();
    }

private:
    QThreadStorage<std::shared_ptr<CURL>> threadCurlStorage;
};

enum class JobOperation {
    UPLOAD = 1,
    DOWNLOAD,
    COPY,
    MOVE,
    DELETE,
    DELETE_LOCAL,
    MKDIR,
    RESUME

};

class TransferManager : public QObject {
    Q_OBJECT
signals:
    void transferStatusUpdated(TransferStatus transferStatus);
   
private:
    DirectoryCache m_DirectoryCache;
   // std::vector<TransferHandle> m_transferHandles;
    std::vector<TransferJob*> m_transferJobs;
    std::string m_username;
    std::string m_password;
    std::string m_url;

    bool m_initialized;
    CurlThreadPool m_threadPool;
    QMutex m_mutex;

private:
    void downloadJob(TransferJob* job);
    void resumeJob(TransferJob* job);
	void uploadJob(TransferJob* job, const std::string& source);
    void copyJob(TransferJob* job, const std::string& source);
    void moveJob(TransferJob* job, const std::string& source, const std::string& destination);
    void deleteJob(TransferJob* job, const std::string& source);
    void deleteLocalJob(TransferJob* job);
	void mkdirJob(TransferJob* job, const std::string& source);

public:
    TransferManager() {}
    TransferManager(QObject* parent)
        : QObject(parent), m_initialized(false) {}

    const TransferJob* getJob(uint64_t  jobId)  const;

    void setCredentials(const std::string& host, const std::string& username, const std::string& password);
    void executeJob(uint64_t jobId, JobOperation jobType, std::shared_ptr<CURL> curl);
    void deleteJob(uint64_t jobId);
    void updateCacheDirectory(const std::string& path) { m_DirectoryCache.updateDirectoryCache(path, 3); }

    void reset();

    [[nodiscard]] const std::vector<DirectoryEntry> getDirectoryList(const std::string& path = "");
    [[nodiscard]] uint64_t prepareJob(const std::string localPath, const std::string remotePath);
    [[nodiscard]] uint64_t prepareJobToResume(const std::string localPath, const std::string remotePath, const uint64_t bytesTransferred, const uint64_t totalBytes, const uint64_t jobId, const TransferStatus::TransferOperation operation);
    [[nodiscard]] bool isInitialized() const { return m_initialized; }
    [[nodiscard]] const std::map<std::string, std::vector<DirectoryEntry>>& getCache() const { return m_DirectoryCache.getCache(); }
    [[nodiscard]] const std::string& getUsername() { return m_username; }
    [[nodiscard]] bool isRegularFile(const std::string& filePath) { return m_DirectoryCache.isRegularFile(filePath); }
    [[nodiscard]] const DirectoryCache* getDirectoryCacheObject() const { return &m_DirectoryCache; }
    

    void connect(const std::string& host, const std::string& username, std::string& password);

    void submitJob(uint64_t jobId, JobOperation jobType);
    void cancelJob(uint64_t jobId);
    void pauseJob(uint64_t jobId);
    void resumeJob(const std::string& localPath, const std::string& remotePath, const uint64_t bytesTransferred, const uint64_t totalBytes, const uint64_t jobId,
                    const TransferStatus::TransferOperation operation);

    ~TransferManager();

public slots:
    void onTransferStatusReceived(TransferStatus status);
};

class JobRunnable : public QRunnable {
private:
    TransferManager* m_transferManager;
    uint64_t m_jobId;
    JobOperation m_jobType;
    CurlThreadPool* m_threadPool;

public:
    JobRunnable(QPointer<TransferManager> transferManager, uint64_t jobId, JobOperation jobType, CurlThreadPool* threadPool)
        : m_transferManager(transferManager), m_jobId(jobId), m_jobType(jobType), m_threadPool(threadPool) {}


    void run() override {
        if (m_transferManager) {
            auto curl = m_threadPool->getCurlForCurrentThread();
            std::cout << "COUNT IN RUN: " << curl.use_count() << std::endl;
            m_transferManager->executeJob(m_jobId, m_jobType, curl);
            std::cout << "COUNT AFTER EXECUTE JOB: " << curl.use_count() << std::endl;
            m_transferManager->deleteJob(m_jobId);

        }
    }
};
#endif //SFTP_INTERFACE_TRANSFERMANAGER_H
