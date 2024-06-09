#ifndef SFTP_INTERFACE_TRANSFERHANDLE_H
#define SFTP_INTERFACE_TRANSFERHANDLE_H

#include <string>
#include "TransferStatus.h"
#include <curl/curl.h>
#include <memory>

class TransferHandle {
private:
    friend class TransferJob;
    std::shared_ptr<CURL> m_curlHandle;
    TransferStatus m_transferStatus;

public:
    TransferHandle();

    explicit TransferHandle(std::shared_ptr<CURL>& curlHandle);
    explicit TransferHandle(std::shared_ptr<CURL>&& curlHandle);

    [[nodiscard]] const TransferStatus &getTransferStatus() const {return m_transferStatus;}

    void setTransferStatus(const TransferStatus &transferStatus) {m_transferStatus = transferStatus;}
    void setBytesTransfered(size_t number){m_transferStatus.m_bytesTransferred += number;}
};

#endif //SFTP_INTERFACE_TRANSFERHANDLE_H
