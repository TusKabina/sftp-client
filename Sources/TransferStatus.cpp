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
    m_state = TransferState::Unknown;
}
