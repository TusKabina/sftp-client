#include "TransferStatus.h"

std::ostream &operator<<(std::ostream& os, TransferStatus::TransferState state) {
    switch (state) {
        case TransferStatus::TransferState::Initialized:
            os << "Initialized";
            break;
        case TransferStatus::TransferState::Pending:
            os << "Pending";
            break;
        case TransferStatus::TransferState::InProgress:
            os << "In Progress";
            break;
        case TransferStatus::TransferState::Completed:
            os << "Completed";
            break;
        case TransferStatus::TransferState::Failed:
            os << "Failed";
            break;
        case TransferStatus::TransferState::Canceled:
            os << "Canceled";
            break;
        case TransferStatus::TransferState::Unknown:
            os << "Unknown";
        default:
            os << "Invalid State";
            break;
    }
    return os;
}

TransferStatus::TransferStatus() {
    m_bytesTransferred = 0;
    m_totalBytes = 0;
    m_errorMessage = "";
    m_curlResCode = -1;
    m_jobId = 0;
    m_state = TransferState::Unknown;
    m_speed = 0.0;
    m_smoothedSpeed = 0.0;
    m_lastUpdateTime = QDateTime::currentDateTime();
    m_lastBytesTransferred = 0;
    m_alpha = 0.1;
    m_lowSpeedCount = 0;
    m_highSpeedCount = 0;
    m_thresholdCount = 5;
    signal_threshold = static_cast<size_t>(1024) * 1024 * 2; // after how many MB to trigger signal for updating transfer status
    m_threshold = 0;
    m_lastUpdateTime = QDateTime::currentDateTime();
}

void TransferStatus::updateSpeed(size_t bytesTransferred) {
    QDateTime now = QDateTime::currentDateTime();
    qint64 durationSinceLastUpdate = m_lastUpdateTime.msecsTo(now);
    qint64 totalDuration = m_startTime.msecsTo(now);

    if (totalDuration > 0) { 
        m_speed = (bytesTransferred / 1024.0 / 1024.0) / (totalDuration / 1000.0); // MB/S

        m_smoothedSpeed = m_alpha * m_speed + (1 - m_alpha) * m_smoothedSpeed;

        if (m_smoothedSpeed < m_speed) {
            m_lowSpeedCount++;
            m_highSpeedCount = 0;
        }
        else {
            m_highSpeedCount++;
            m_lowSpeedCount = 0;
        }

        if (m_lowSpeedCount >= m_thresholdCount || m_highSpeedCount >= m_thresholdCount) {
            m_speed = m_smoothedSpeed;
            m_lowSpeedCount = 0;
            m_highSpeedCount = 0;
        }
    }

    m_lastUpdateTime = now;
    m_lastBytesTransferred = bytesTransferred;
}

const std::string TransferStatus::TransferStatetoString() const {
    std::string strState;
    switch (m_state) {
    case TransferStatus::TransferState::Initialized:
        strState = "Initialized";
        break;
    case TransferStatus::TransferState::Pending:
        strState = "Pending";
        break;
    case TransferStatus::TransferState::InProgress:
        strState = "In Progress";
        break;
    case TransferStatus::TransferState::Completed:
        strState = "Completed";
        break;
    case TransferStatus::TransferState::Failed:
        strState = "Failed";
        break;
    case TransferStatus::TransferState::Canceled:
        strState = "Canceled";
        break;
    case TransferStatus::TransferState::Unknown:
        strState = "Unknown";
    default:
        strState = "Invalid State";
        break;
    }
    return strState;
}
