#ifndef SFTP_INTERFACE_TRANSFERSTATUS_H
#define SFTP_INTERFACE_TRANSFERSTATUS_H

#include <ostream>
#include <QDateTime>

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
    std::string m_source;
    std::string m_destination;
    std::string m_errorMessage;
    int m_curlResCode;
    uint64_t m_jobId;
    double m_speed;
    double m_smoothedSpeed;
    size_t m_threshold;
    size_t signal_threshold;

    QDateTime m_lastUpdateTime;
    size_t m_lastBytesTransferred;

    // For smoothing and filtering
    double m_alpha;
    int m_lowSpeedCount;
    int m_highSpeedCount;
    int m_thresholdCount;

    TransferStatus();
    void updateSpeed(size_t bytesTransferred);
};
inline std::ostream& operator<<(std::ostream& os, TransferStatus::TransferState state);

#endif //SFTP_INTERFACE_TRANSFERSTATUS_H
