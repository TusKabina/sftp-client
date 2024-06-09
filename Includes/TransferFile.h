#ifndef SFTP_INTERFACE_TRANSFERFILE_H
#define SFTP_INTERFACE_TRANSFERFILE_H
#include <string>

struct TransferFile {
    std::string m_localPath;
    std::string m_localDirectoryPath;
    std::string m_remotePath;
    std::string m_remoteDirectoryPath;
    uint64_t m_bytesTransfered = 0;
    uint64_t m_totalBytes = 0;
    FILE* m_stream = nullptr;
};
#endif //SFTP_INTERFACE_TRANSFERFILE_H
