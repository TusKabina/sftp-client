#ifndef SFTP_INTERFACE_TRANSFERJOB_H
#define SFTP_INTERFACE_TRANSFERJOB_H

#include "TransferHandle.h"
#include "TransferFile.h"
#include "UIDGenerator.h"
#include <vector>
class TransferJob {
private:
    TransferHandle m_transferHandle;
    TransferFile m_transferFile;
    std::string m_url;
    uint64_t m_jobId = 0;
public:
    TransferJob() {}

    [[nodiscard]] const TransferStatus& getTransferStatus() const {return m_transferHandle.m_transferStatus;}
    [[nodiscard]] const std::string& getLocalDirectoryPath() const { return m_transferFile.m_localDirectoryPath; }
    [[nodiscard]] const std::string& getRemoteDirectoryPath() const { return m_transferFile.m_remoteDirectoryPath; }
    [[nodiscard]] const std::string& getLocalPath() const { return m_transferFile.m_localPath;}
    [[nodiscard]] const std::string& getRemotePath() const {return m_transferFile.m_remotePath;}
    [[nodiscard]] const std::string& getUrl() const {return m_url;}
    [[nodiscard]] uint64_t getJobId() const {return m_jobId;}
    [[nodiscard]] uint64_t getBytesTransferred() const {return m_transferFile.m_bytesTransfered;}

    void setLocalPath(const std::string& localPath) {m_transferFile.m_localPath = localPath;}
    void setRemotePath(const std::string& remotePath) {m_transferFile.m_remotePath = remotePath;}
    void setUrl(const std::string& url){m_url = url;}
    void setJobId(uint64_t jobId) {m_jobId = jobId;}
    void setTransferHandle(std::shared_ptr<CURL>& curlHandle) {m_transferHandle = TransferHandle(curlHandle);}
    void closeStreamFile();

    uint64_t createJob(TransferHandle& job, const std::string& localPath, const std::string& remotePath);

    ~TransferJob();

private:
    static size_t WriteCallback(void* buffer, size_t size, size_t nmemb, TransferFile* transferFile);
    static size_t dummyWriteCallback(void* ptr, size_t size, size_t nmemb, void* stream);
    static size_t ReadCallback(void *buffer, size_t size, size_t nmemb, TransferFile* transferFile);

public:
    void downloadFile(const std::string& url);
    void uploadFile(const std::string& url);
    void copyFile(const std::string& url);
    void moveFile(const std::string& url);
    void deleteFile(const std::string& url);
};

#endif //SFTP_INTERFACE_TRANSFERJOB_H
