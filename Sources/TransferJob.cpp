#include "TransferJob.h"
#include <iostream>

size_t TransferJob::WriteCallback(void* buffer, size_t size, size_t nmemb, TransferFile* transferFile) {
    if (transferFile->m_stream) {
        size_t totalSize = size * nmemb;
        size_t bytesWritten = fwrite(buffer, 1, totalSize, transferFile->m_stream);
        transferFile->m_totalBytes = totalSize;
        transferFile->m_bytesTransfered += bytesWritten;
        return bytesWritten;
    } else {
        return 0;
    }
}

size_t TransferJob::dummyWriteCallback(void* ptr, size_t size, size_t nmemb, void* stream) {
    return size * nmemb;
}

size_t TransferJob::ReadCallback(void* buffer, size_t size, size_t nmemb, TransferFile* transferFile) {
    if (transferFile->m_stream) {
        size_t totalSize = size * nmemb;
        size_t bytesRead = fread(buffer, 1, totalSize, transferFile->m_stream);
        transferFile->m_totalBytes = totalSize;
        transferFile->m_bytesTransfered += bytesRead;
        return bytesRead;
    }
    return 0;
}

void TransferJob::downloadFile(const std::string& url) {
    if (m_transferHandle.m_curlHandle.get()) {
        m_transferFile.m_stream = fopen(m_transferFile.m_localPath.c_str(), "wb");
        if (!m_transferFile.m_stream) {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Failed to open file for writing!";
            return;
        }

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, (url + m_transferFile.m_remotePath).c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEFUNCTION, TransferJob::WriteCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEDATA, &m_transferFile);

        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());

        if (res != CURLE_OK) {
            m_transferHandle.m_transferStatus.m_curlResCode = (int)res;
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Curl easy perform error: " + std::string(curl_easy_strerror(res));
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
        }
        closeStreamFile();
    }
}
void TransferJob::uploadFile(const std::string& url) {
    if (m_transferHandle.m_curlHandle.get()) {
        m_transferFile.m_stream = fopen(m_transferFile.m_localPath.c_str(), "rb");
        if (!m_transferFile.m_stream)
        {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Failed to open file for reading!";
            return;
        }

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, (url + m_transferFile.m_remotePath).c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_READFUNCTION, TransferJob::ReadCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_READDATA, &m_transferFile);

        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());

        if (res != CURLE_OK) {
            
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Curl easy perform error: " + std::string(curl_easy_strerror(res));
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
        }
        //TODO: make custom deleter for CURL shared_ptr where on each decrement curl_easy_reset will be called
        curl_easy_reset(m_transferHandle.m_curlHandle.get());
        closeStreamFile();
    }
}

void TransferJob::copyFile(const std::string& url) {

   
}

void TransferJob::moveFile(const std::string& url) {
    if (m_transferHandle.m_curlHandle.get()) {
        struct curl_slist* header = NULL;
        std::string renameCommand = "rename " + m_transferFile.m_localPath + " " + m_transferFile.m_remotePath;
        header = curl_slist_append(header, renameCommand.c_str());

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEFUNCTION, dummyWriteCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_QUOTE, header);
        
        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());
        
        if (res != CURLE_OK) {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Curl easy perform error: " + std::string(curl_easy_strerror(res));
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
        }
        curl_slist_free_all(header);
    }
}

void TransferJob::deleteFile(const std::string& url) {
    if (m_transferHandle.m_curlHandle.get()) {
        struct curl_slist* header = NULL;
        std::string renameCommand = "rm " + m_transferFile.m_remotePath;
        header = curl_slist_append(header, renameCommand.c_str());

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEFUNCTION, dummyWriteCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_QUOTE, header);

        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());

        if (res != CURLE_OK) {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Curl easy perform error: " + std::string(curl_easy_strerror(res));
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
        }
        curl_slist_free_all(header);
    }
}

uint64_t TransferJob::createJob(TransferHandle& transferHandle, const std::string& localPath, const std::string& remotePath) {
    m_transferHandle  = transferHandle;
    m_transferFile.m_localPath = localPath;
    m_transferFile.m_localDirectoryPath = m_transferFile.m_localPath.substr(0, m_transferFile.m_localPath.find_last_of('/'));
    m_transferFile.m_remotePath = remotePath;
    m_transferFile.m_remoteDirectoryPath = m_transferFile.m_remotePath.substr(0, m_transferFile.m_remotePath.find_last_of('/'));
    m_jobId = UIDGenerator::getInstance().generateID();
    return m_jobId;
}

TransferJob::~TransferJob() {
    closeStreamFile();
}

void TransferJob::closeStreamFile() {
    if (m_transferFile.m_stream) {
        fclose(m_transferFile.m_stream);
        m_transferFile.m_stream = nullptr;
    }
}