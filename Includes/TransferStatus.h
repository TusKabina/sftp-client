#ifndef SFTP_INTERFACE_TRANSFERSTATUS_H
#define SFTP_INTERFACE_TRANSFERSTATUS_H

#include <ostream>
struct TransferStatus {
    enum class TransferState {
        Initialized,
        Pending,
        InProgress,
        Completed,
        Failed,
        Canceled,
        Unknown
    };

    TransferState m_state;
    size_t m_bytesTransferred;
    size_t m_totalBytes;
    std::string m_errorMessage;
    int m_curlResCode;
    uint64_t m_jobId;
    TransferStatus();
};
inline std::ostream& operator<<(std::ostream& os, TransferStatus::TransferState state);

#endif //SFTP_INTERFACE_TRANSFERSTATUS_H
