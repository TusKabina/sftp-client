#include "TransferJob.h"
#include <iostream>
#include <thread>
#include <cstdio>
#include "Utilities/Logger.h"

std::string urlEncoder(const std::string& url) {
    std::ostringstream encoded;
    for (unsigned char c : url) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
            encoded << c;
        }
        else {
            encoded << '%' << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(c);
        }
    }
    return encoded.str();
}

size_t TransferJob::WriteCallback(void* buffer, size_t size, size_t nmemb, void* parent) {
    TransferJob* job = static_cast<TransferJob*>(parent);
    if (job->m_transferFile.m_stream) {
        size_t totalSize = size * nmemb;
        size_t bytesWritten = fwrite(buffer, 1, totalSize, job->m_transferFile.m_stream);

        job->m_transferHandle.m_transferStatus.updateSpeed(job->m_transferHandle.m_transferStatus.m_bytesTransferred);
        job->m_transferHandle.m_transferStatus.m_bytesTransferred += bytesWritten;
        job->m_transferHandle.m_transferStatus.m_threshold += bytesWritten;

        if (job->m_transferHandle.m_transferStatus.m_threshold >= job->m_transferHandle.m_transferStatus.signal_threshold) {
            job->m_transferHandle.m_transferStatus.m_threshold = 0;
            job->m_transferHandle.m_transferStatus.m_progress = (static_cast<double>(job->m_transferHandle.m_transferStatus.m_bytesTransferred) / 
                job->m_transferHandle.m_transferStatus.m_totalBytes) * 100;

            job->onTransferStatusUpdated(job->m_transferHandle.m_transferStatus);
        }
        return bytesWritten;
    } 
    else {
        return 0;
    }
}

size_t TransferJob::dummyWriteCallback(void* ptr, size_t size, size_t nmemb, void* stream) {
    return size * nmemb;
}

size_t TransferJob::ReadCallback(void* buffer, size_t size, size_t nmemb, void* parent) {
    TransferJob* job = static_cast<TransferJob*>(parent);
    if (job->m_transferFile.m_stream) {
        size_t totalSize = size * nmemb;
        size_t bytesRead = fread(buffer, 1, totalSize, job->m_transferFile.m_stream);
        job->m_transferHandle.m_transferStatus.updateSpeed(job->m_transferHandle.m_transferStatus.m_bytesTransferred);
        job->m_transferHandle.m_transferStatus.m_bytesTransferred += bytesRead;
        job->m_transferHandle.m_transferStatus.m_threshold += bytesRead;

        if (job->m_transferHandle.m_transferStatus.m_threshold >= job->m_transferHandle.m_transferStatus.signal_threshold) {
            job->m_transferHandle.m_transferStatus.m_threshold = 0;
            job->m_transferHandle.m_transferStatus.m_progress = (static_cast<double>(job->m_transferHandle.m_transferStatus.m_bytesTransferred) /
                job->m_transferHandle.m_transferStatus.m_totalBytes) * 100;
            job->onTransferStatusUpdated(job->m_transferHandle.m_transferStatus);
        }
        return bytesRead;
    }
    return 0;
}

void TransferJob::downloadFile() {
    if (m_transferHandle.m_curlHandle.get()) {

        m_transferFile.m_stream = fopen(m_transferFile.m_localPath.c_str(), "wb");
        if (!m_transferFile.m_stream) {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[DOWNLOAD] ERROR: Failed to open file for writing!";
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
            return;
        }

        std::string encodedUrl = urlEncoder(m_transferFile.m_remotePath);

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, (m_url + encodedUrl).c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_BUFFERSIZE, 131072L); // 128KB
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEFUNCTION, TransferJob::WriteCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEDATA, this);

        m_transferHandle.m_transferStatus.m_startTime = QDateTime::currentDateTime();
        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;

        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());
       
        if (res != CURLE_OK) {
            m_transferHandle.m_transferStatus.m_curlResCode = (int)res;
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[DOWNLOAD] ERROR: Curl easy perform error: " + std::string(curl_easy_strerror(res)) + " LocalPath: " + m_transferFile.m_localPath;
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[DOWNLOAD] Download source: " + m_transferFile.m_remotePath + " is finished";
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
        }
        closeStreamFile();
        curl_easy_reset(m_transferHandle.m_curlHandle.get());

        m_transferHandle.m_transferStatus.m_progress = (static_cast<double>(m_transferHandle.m_transferStatus.m_bytesTransferred) /
            m_transferHandle.m_transferStatus.m_totalBytes) * 100;

        onTransferStatusUpdated(m_transferHandle.m_transferStatus);
    }
}
void TransferJob::uploadFile(const std::string& url) {
    if (m_transferHandle.m_curlHandle.get()) {
        m_transferFile.m_stream = fopen(m_transferFile.m_localPath.c_str(), "rb");
        if (!m_transferFile.m_stream)
        {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Failed to open file for reading!";
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
            return;
        }

        std::string encodedUrl = urlEncoder(m_transferFile.m_remotePath);
       
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, (m_url + encodedUrl).c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_UPLOAD_BUFFERSIZE, 131072L); // 128KB
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_READFUNCTION, TransferJob::ReadCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_READDATA, this);
       
        m_transferHandle.m_transferStatus.m_startTime = QDateTime::currentDateTime();
        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        std::cout << "[UPLOAD] JOBID: " << m_jobId << " starting curl_easy_perform: " << "\n";
        std::cout << "[UPLOAD] REMOTE PATH: " << m_transferFile.m_remotePath << "\n";
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());

        if (res != CURLE_OK) {
            
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[UPLOAD] Error Message: Curl easy perform error: " + std::string(curl_easy_strerror(res));
            std::cout << "[UPLOAD] JOBID: " << m_jobId << " starting curl_easy_perform: " << m_transferHandle.m_transferStatus.m_errorMessage << "\n";
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
           
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[UPLOAD] Upload source: " + m_transferFile.m_localPath + " is finished";
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
        }
        //TODO: make custom deleter for CURL shared_ptr where on each decrement curl_easy_reset will be called
        closeStreamFile();
        curl_easy_reset(m_transferHandle.m_curlHandle.get());

        m_transferHandle.m_transferStatus.m_progress = (static_cast<double>(m_transferHandle.m_transferStatus.m_bytesTransferred) /
            m_transferHandle.m_transferStatus.m_totalBytes) * 100;

        onTransferStatusUpdated(m_transferHandle.m_transferStatus);
    }
}

void TransferJob::copyFile() {
    if (m_transferHandle.m_curlHandle.get()) {
        std::string remotePathDirectory = m_transferFile.m_localDirectoryPath + "/";
        std::string remoteFileName = m_transferFile.m_localPath.substr(remotePathDirectory.size(), m_transferFile.m_localPath.size());
        m_transferFile.m_stream = fopen((remoteFileName).c_str(), "wb");
        if (!m_transferFile.m_stream) {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Failed to open file for writing!";
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
            return;
        }

        std::string encodedUrl = urlEncoder(m_transferFile.m_localPath);

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, (m_url + encodedUrl).c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEFUNCTION, TransferJob::WriteCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEDATA, this);

        m_transferHandle.m_transferStatus.m_startTime = QDateTime::currentDateTime();
        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());

        if (res != CURLE_OK) {
            m_transferHandle.m_transferStatus.m_curlResCode = (int)res;
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[DOWNLOAD] Error Message: Curl easy perform error: " + std::string(curl_easy_strerror(res)) + " RemotePath: " + m_transferFile.m_localPath;
            std::cout << "[DOWNLOAD] Error Message: " + m_transferHandle.m_transferStatus.m_errorMessage << std::endl;
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);

            closeStreamFile();
            curl_easy_reset(m_transferHandle.m_curlHandle.get());
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
            closeStreamFile();
            curl_easy_reset(m_transferHandle.m_curlHandle.get());

            m_transferFile.m_localDirectoryPath = "";
            m_transferFile.m_localPath = remoteFileName;
            m_transferHandle.m_transferStatus.m_errorMessage = "[DOWNLOAD] Download source: " + m_transferFile.m_remotePath + " is finished. Starting upload";
            m_transferHandle.m_transferStatus.m_lastUpdateTime = QDateTime::currentDateTime();
            m_transferHandle.m_transferStatus.m_speed = 0;
            m_transferHandle.m_transferStatus.m_smoothedSpeed = 0;
            m_transferHandle.m_transferStatus.m_bytesTransferred = 0;

            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
            uploadFile(m_url);
        }
        deleteLocalFile(m_transferFile.m_localPath);
    }
   
}

void TransferJob::moveFile(const std::string& url) {
    if (m_transferHandle.m_curlHandle.get()) {
        struct curl_slist* header = NULL;
        // std::string encodedLocalPath = urlEncoder(m_transferFile.m_localPath);
        // std::string encodedRemotePath = urlEncoder(m_transferFile.m_remotePath);

        std::string renameCommand = "rename " + std::string("\"") + m_transferFile.m_localPath + std::string("\"") + " "
            + std::string("\"") + m_transferFile.m_remotePath + std::string("\"");
        header = curl_slist_append(header, renameCommand.c_str());

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEFUNCTION, dummyWriteCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_QUOTE, header);
        
        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());
        
        if (res != CURLE_OK) {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[MOVE] Error Message: Failed to move file " + m_transferFile.m_localPath;
            std::cout << "[MOVE] Error Message: " + m_transferHandle.m_transferStatus.m_errorMessage << std::endl;
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[MOVE] Move source: " + m_transferFile.m_localPath + " is finished";
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
        }
        curl_slist_free_all(header);
    }
    curl_easy_reset(m_transferHandle.m_curlHandle.get());
}

void TransferJob::deleteFile(const std::string& url) {
    if (m_transferHandle.m_curlHandle.get()) {
        struct curl_slist* header = NULL;
        //std::string encodedRemotePath = urlEncoder(m_transferFile.m_remotePath);
        std::string renameCommand = "rm " + std::string("\"") + m_transferFile.m_remotePath + std::string("\"");
        std::cout << "RENAME COMMAND: " << renameCommand << std::endl;
        header = curl_slist_append(header, renameCommand.c_str());

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, m_url.c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEFUNCTION, dummyWriteCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_QUOTE, header);

        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        std::cout << "[DELETE] JOBID: " << m_jobId << " starting curl_easy_perform: " << "\n";
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());

        if (res != CURLE_OK) {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[DELETE] Error Message: Failed to delete a file " + m_transferFile.m_localPath;
            std::cout << "[DELETE] Error Message: " + m_transferHandle.m_transferStatus.m_errorMessage << std::endl;
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
            m_transferHandle.m_transferStatus.m_errorMessage = "[DELETE] Delete file: " + m_transferFile.m_remotePath + " completed.";
            onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
        }
        curl_slist_free_all(header);
    }
    curl_easy_reset(m_transferHandle.m_curlHandle.get());
}

void TransferJob::deleteLocalFile(const std::string& path) {
    if (std::remove(path.c_str()) == 0) {
        std::cout << "File successfully deleted\n";
        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
    }
    else {
        std::perror("Error deleting file");
        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
        m_transferHandle.m_transferStatus.m_errorMessage = "[DELETE_LOCAL] Error Message: Error deleting file";
        std::cout << "[DELETE_LOCAL] Error Message: " + m_transferHandle.m_transferStatus.m_errorMessage << std::endl;
        onErrorMessage(m_transferHandle.m_transferStatus.m_errorMessage);
    }
}

uint64_t TransferJob::createJob(const std::string localPath, const std::string remotePath, const std::string url) {
    m_transferFile.m_localPath = localPath;
    m_transferFile.m_localDirectoryPath = m_transferFile.m_localPath.substr(0, m_transferFile.m_localPath.find_last_of('/'));
    m_transferFile.m_remotePath = remotePath;
    m_transferFile.m_remoteDirectoryPath = m_transferFile.m_remotePath.substr(0, m_transferFile.m_remotePath.find_last_of('/'));
    m_jobId = UIDGenerator::getInstance().generateID();
    m_url = url;
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

