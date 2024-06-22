#include "TransferJob.h"
#include <iostream>
#include <thread>
#include <cstdio>

size_t TransferJob::WriteCallback(void* buffer, size_t size, size_t nmemb, void* parent) {
    TransferJob* job = static_cast<TransferJob*>(parent);
    if (job->m_transferFile.m_stream) {
        size_t totalSize = size * nmemb;
        size_t bytesWritten = fwrite(buffer, 1, totalSize, job->m_transferFile.m_stream);

        job->m_transferHandle.m_transferStatus.m_bytesTransferred += bytesWritten;

        job->onTransferStatusUpdated(job->m_transferHandle.m_transferStatus);
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

void TransferJob::downloadFile() {
    if (m_transferHandle.m_curlHandle.get()) {

        m_transferFile.m_stream = fopen(m_transferFile.m_localPath.c_str(), "wb");
        if (!m_transferFile.m_stream) {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Failed to open file for writing!";
            return;
        }

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, (m_url + m_transferFile.m_remotePath).c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEFUNCTION, TransferJob::WriteCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_WRITEDATA, this);

        std::cout << "---------------------------------UNUTAR DOWNLOAD---------------------------------------------------" << std::endl;
        std::cout << "REMOTEPATH: " << m_transferFile.m_remotePath << std::endl;
        std::cout << "LOCALPATH: " << m_transferFile.m_localPath << std::endl;
        std::cout << "JOBID " << m_jobId << std::endl;
        std::cout << "---------------------------------UNUTAR DOWNLOAD---------------------------------------------------" << std::endl;

        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());
        std::thread::id this_id = std::this_thread::get_id();
        std::cout << "THREAD ID " << this_id << "OVO JE THREAD ID!!!!!!!!!!!!!!!!!!!!!!!\n";
        std::cout << "CURL ADDRESS: " << (m_transferHandle.m_curlHandle.get()) << std::endl;
        std::cout << "---------------------------------NAKON PERFORME---------------------------------------------------" << std::endl;
        std::cout << "REMOTEPATH: " << m_transferFile.m_remotePath << std::endl;
        std::cout << "LOCALPATH: " << m_transferFile.m_localPath << std::endl;
        std::cout << "JOBID " << m_jobId << std::endl;
        std::cout << "---------------------------------NAKON PERFORME---------------------------------------------------" << std::endl;

        if (res != CURLE_OK) {
            m_transferHandle.m_transferStatus.m_curlResCode = (int)res;
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Curl easy perform error: " + std::string(curl_easy_strerror(res)) + " LocalPath: " + m_transferFile.m_localPath;
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
        }
        closeStreamFile();
        curl_easy_reset(m_transferHandle.m_curlHandle.get());
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
            return;
        }

        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_URL, (url + m_transferFile.m_remotePath).c_str());
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_READFUNCTION, TransferJob::ReadCallback);
        curl_easy_setopt(m_transferHandle.m_curlHandle.get(), CURLOPT_READDATA, &m_transferFile);

        m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::InProgress;
        std::cout << "[UPLOAD] JOBID: " << m_jobId << " starting curl_easy_perform: " << "\n";
        std::cout << "[UPLOAD] REMOTE PATH: " << m_transferFile.m_remotePath << "\n";
        CURLcode res = curl_easy_perform(m_transferHandle.m_curlHandle.get());

        if (res != CURLE_OK) {
            
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Failed;
            m_transferHandle.m_transferStatus.m_errorMessage = "Curl easy perform error: " + std::string(curl_easy_strerror(res));
            std::cout << "[UPLOAD] JOBID: " << m_jobId << " starting curl_easy_perform: " << m_transferHandle.m_transferStatus.m_errorMessage << "\n";
        }
        else {
            m_transferHandle.m_transferStatus.m_state = TransferStatus::TransferState::Completed;
        }
        //TODO: make custom deleter for CURL shared_ptr where on each decrement curl_easy_reset will be called
        closeStreamFile();
        curl_easy_reset(m_transferHandle.m_curlHandle.get());
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
    curl_easy_reset(m_transferHandle.m_curlHandle.get());
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
        std::cout << "[DELETE] JOBID: " << m_jobId << " starting curl_easy_perform: " << "\n";
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
        m_transferHandle.m_transferStatus.m_errorMessage = "Error deleting file";
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

