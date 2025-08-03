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
        Cancelled,
        Paused,
        Unknown
    };
    // Used for displaying the operation type in the UI (only download, upload, and copy)
    enum class TransferOperation {
        Download,
        Upload,
        Copy,
		Unknown
    };
    TransferState m_state;
    size_t m_bytesTransferred;
    size_t m_totalBytes;
    size_t m_threshold;
    size_t m_signalThreshold;
    size_t m_lastBytesTransferred;
    uint64_t m_jobId;
    
    std::string m_source;
    std::string m_destination;
    std::string m_errorMessage;
	// Used for displaying the operation type in the UI (only download, upload, and copy)
    TransferOperation m_operation;
    
    int m_curlResCode;
    double m_speed;
    double m_smoothedSpeed;
    double m_progress;

    QDateTime m_startTime;
    QDateTime m_lastUpdateTime;

    // Smoothing and filtering download speed
    double m_alpha;
    int m_lowSpeedCount;
    int m_highSpeedCount;
    int m_thresholdCount;

    TransferStatus();
    void updateSpeed(size_t bytesTransferred);
    void reset();
    const std::string TransferStatetoString() const;
    const std::string transferOperationToString() const;
	static const TransferOperation transferOperationFromString(const std::string& operation);
};
inline std::ostream& operator<<(std::ostream& os, TransferStatus::TransferState state);

#endif //SFTP_INTERFACE_TRANSFERSTATUS_H
