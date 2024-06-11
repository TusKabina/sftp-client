#ifndef SFTP_INTERFACE_TRANSFERMANAGER_H
#define SFTP_INTERFACE_TRANSFERMANAGER_H
#include <vector>
#include <map>
#include <string>
#include "TransferJob.h"
#include "DirectoryCache.h"
#include "HandleDeleter.h"

enum class JobOperation {
    UPLOAD = 1,
    DOWNLOAD,
    COPY,
    MOVE,
    DELETE

};
class TransferManager {
private:
    DirectoryCache m_DirectoryCache;
    std::vector<TransferHandle> m_transferHandles;
    std::vector<TransferJob> m_transferJobs;
    std::string m_username;
    std::string m_password;
    std::string m_url;
    uint64_t m_maxHandlesNumber;
    int m_handleCounter = 0;
    bool m_initialized;

public:

    //TransferManager(const std::string& host, const std::string& username, std::string& password);
    TransferManager() = default;

    TransferHandle& findFreeHandle();
    const TransferJob& getJob(uint64_t  jobId)  const;

    void setCredentials(const std::string& host, const std::string& username, const std::string& password);
    void executeJob(uint64_t jobId, JobOperation jobType);
    void deleteJob(uint64_t jobId);

    [[nodiscard]] const std::vector<DirectoryEntry> getDirectoryList(const std::string& path = "");
    [[nodiscard]] uint64_t prepareJob(const std::string& localPath, const std::string& remotePath);
    [[nodiscard]] bool isInitialized() const { return m_initialized; }
    [[nodiscard]] const std::map<std::string, std::vector<DirectoryEntry>>& getCache() const { return m_DirectoryCache.getCache(); }
    [[nodiscard]] const std::string& getUsername() { return m_username; }
    void connect(const std::string& host, const std::string& username, std::string& password);
    bool isRegularFile(const std::string& filePath) { return m_DirectoryCache.isRegularFile(filePath); }
    ~TransferManager();
};
#endif //SFTP_INTERFACE_TRANSFERMANAGER_H
