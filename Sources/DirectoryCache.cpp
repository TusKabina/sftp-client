#include "DirectoryCache.h"
#include <iostream> //TODO: DELETE
#include <sstream>
#include <algorithm>
bool DirectoryCache::initialize(const std::string& host, const std::string& username, std::string& password) {
	m_curlHandle = CurlUniquePtr(curl_easy_init());
    m_curlCode = 0;
	if (m_curlHandle) {
        char* encodedPassword = curl_easy_escape(m_curlHandle.get(), password.c_str(), 0);
        password = std::string(encodedPassword);
        curl_free(encodedPassword);

	    m_baseUrl = "sftp://" + username + ":" + password + "@" + host;

        curl_easy_setopt(m_curlHandle.get(), CURLOPT_URL, m_baseUrl.c_str());
        curl_easy_setopt(m_curlHandle.get(), CURLOPT_WRITEFUNCTION, dummyWriteCallback);

        CURLcode res = curl_easy_perform(m_curlHandle.get());
        if (res != 0) {
            m_curlCode = static_cast<int>(res);
            m_initialized = false;
        }
        else {
            m_initialized = true;
        }
        curl_easy_reset(m_curlHandle.get());
    }
    else {
        m_initialized = false;
    }
    return m_initialized;
}


void DirectoryCache::prefetchDirectories(const std::string& path, int depth) {
    if (depth == 0) {
        return;
    }

    std::vector<DirectoryEntry> entries = listDirectory(path);
    m_cache[path] = entries;

    for (const auto& entry : entries) {
        if (entry.m_isDirectory && (entry.m_name != ".." && entry.m_name != ".")) {
            std::string subPath;
            subPath = path + entry.m_name + "/";
            prefetchDirectories(subPath, depth - 1);
        }
    }
}

std::vector<DirectoryEntry> DirectoryCache::listDirectory(const std::string& path)
{
    std::vector<DirectoryEntry> entries;
    
    if (!m_curlHandle.get()) {
        return entries;
    }
    std::string fullUrl = m_baseUrl + path;
    std::string response;

    curl_easy_setopt(m_curlHandle.get(), CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(m_curlHandle.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(m_curlHandle.get(), CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(m_curlHandle.get());

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        m_curlCode = static_cast<int>(res);
        return entries;
    }
    parseResponse(entries, response);
    return entries;
}

bool DirectoryCache::getCachedDirectory(const std::string& path, std::vector<DirectoryEntry>& entries) const {
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        entries = it->second;
        return true;
    }
    return false;
}

bool DirectoryCache::isRegularFile(const std::string& filePath) {
   /* size_t pos = filePath.find_last_of("/");
    std::string fileName = filePath.substr(0, pos + 1);
    if (isPathInCache(filePath + "/")) {
        return false;
    }
    if (!isPathInCache(fileName + "/")) {
        return false;
    }
    std::vector<DirectoryEntry> entries = m_cache.at(filePath + "/");
    auto file = std::find_if(entries.begin(), entries.end(), [&fileName](const DirectoryEntry& entry) {
        return entry.m_name == fileName; });

    return file != entries.end() && file->m_isDirectory;*/

    return !isPathInCache(filePath + "/");
}

void DirectoryCache::refreshDirectory(const std::string& path) {
    std::vector<DirectoryEntry> entries = listDirectory(path);
    m_cache[path] = entries;
}

size_t DirectoryCache::writeCallback(void* buffer, size_t size, size_t nmemb, std::string& response) {
	response.append(static_cast<char*>(buffer), size * nmemb);
	return size * nmemb;
}

void DirectoryCache::parseResponse(std::vector<DirectoryEntry>& entries, const std::string& response) {
    std::istringstream ss(response);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream lineStream(line);
        std::string permissions, hardLinks, owner, group, sizeStr, month, day, timeOrYear, name;

        lineStream >> permissions >> hardLinks >> owner >> group >> sizeStr >> month >> day >> timeOrYear;
        std::getline(lineStream, name);

        name = name.substr(name.find_first_not_of(' '));

        DirectoryEntry entry;
        entry.m_isDirectory = permissions[0] == 'd' ? true : false;
        entry.m_isSymLink = permissions[0] == 'l' ? true : false;
        entry.m_totalBytes = std::stoul(sizeStr);
        entry.m_name = name;

        entries.push_back(entry);
    }
}
