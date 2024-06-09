#include "TransferHandle.h"

TransferHandle::TransferHandle() {
    m_transferStatus = TransferStatus();
}

TransferHandle::TransferHandle(std::shared_ptr<CURL>& curlHandle) {
    m_curlHandle = curlHandle;
    m_transferStatus = TransferStatus();
    m_transferStatus.m_state = TransferStatus::TransferState::Initialized;
}

TransferHandle::TransferHandle(std::shared_ptr<CURL>&& curlHandle) {
    m_curlHandle = std::move(curlHandle);
    m_transferStatus = TransferStatus();
    m_transferStatus.m_state = TransferStatus::TransferState::Initialized;
}