#include "DirectoryCache.h"
#include <iostream> //TODO: DELETE
#include <sstream>
#include <algorithm>
#include "Utilities/Logger.h"

std::string urlEncode(const std::string& url) {
    std::ostringstream encoded;
    for (unsigned char c : url) {
        // Encode all non-alphanumeric characters except for '-', '_', '.', and '~'
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
            encoded << c;
        }
        else {
            encoded << '%' << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(c);
        }
    }
    return encoded.str();
}

time_t DirectoryCache::parseDateFromLs(const std::string& monthStr, const std::string& dayStr, const std::string& timeOrYearStr) {
    static const std::unordered_map<std::string, int> monthMap = {
        {"Jan", 0}, {"Feb", 1}, {"Mar", 2}, {"Apr", 3},
        {"May", 4}, {"Jun", 5}, {"Jul", 6}, {"Aug", 7},
        {"Sep", 8}, {"Oct", 9}, {"Nov", 10}, {"Dec", 11}
    };

    struct tm tm = { 0 };
    auto monthIt = monthMap.find(monthStr);
    if (monthIt == monthMap.end()) {
        return 0; // Invalid month
    }
    tm.tm_mon = monthIt->second;

    tm.tm_mday = std::stoi(dayStr);

    time_t now = time(nullptr);
    struct tm* now_tm = localtime(&now);

    if (timeOrYearStr.find(':') != std::string::npos) {
        // Time format
        int hour = std::stoi(timeOrYearStr.substr(0, 2));
        int minute = std::stoi(timeOrYearStr.substr(3, 2));
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = 0;
        tm.tm_year = now_tm->tm_year;

        time_t file_time = mktime(&tm);

        // If the computed file_time is more than 6 months in the future,
        // it means the file was actually modified in the previous year
        if (difftime(file_time, now) > (6 * 30 * 24 * 3600)) {
            tm.tm_year -= 1;
            file_time = mktime(&tm);
        }
        return file_time;
    }
    else {
        // Year format
        int year = std::stoi(timeOrYearStr);
        tm.tm_year = year - 1900;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        return mktime(&tm);
    }
}

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
            logger().error() << std::string(curl_easy_strerror(res));
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
    std::vector<DirectoryEntry> entries;
    entries = listDirectory(path);
    if (entries.empty()) {
            return;
    }
    {
       // QMutexLocker locker(&m_mutex);
    }
    m_cache[path] = entries;
    for (const auto& entry : entries) {
        if (entry.m_isDirectory && (entry.m_name != ".." && entry.m_name != "." && entry.m_name != "mnt")) {
            std::string subPath;
            subPath = path + entry.m_name + "/";
            prefetchDirectories(subPath, depth - 1);
        }
    }
    logger().debug() << "Pre fetching directory: " << path;

}

void DirectoryCache::updateDirectoryCache(const std::string& path, int depth) {
    prefetchDirectories(path, depth);
}

std::vector<DirectoryEntry> DirectoryCache::listDirectory(const std::string& path) {
    //QMutexLocker locker(&m_mutex);
    std::vector<DirectoryEntry> entries;
    if (!m_curlHandle.get()) {
        return entries;
    }
    std::string encodedPath = urlEncode(path);
    std::string fullUrl = m_baseUrl + encodedPath;
    std::string response;
   // std::string encodedUrl = urlEncode(fullUrl);


    curl_easy_setopt(m_curlHandle.get(), CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(m_curlHandle.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(m_curlHandle.get(), CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(m_curlHandle.get());

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << " DIRPATH: " << path << std::endl;
        //logger().error() << curl_easy_strerror(res) << ": " << path;
        logger().error() << "Failed to list Directory:" << path << "'. Error: " << std::string(curl_easy_strerror(res));
        m_curlCode = static_cast<int>(res);
        curl_easy_reset(m_curlHandle.get());
        return entries;
    }
    else {
        logger().debug() << "Directory listing of: " << path << " Successful.";
    }
    parseResponse(entries, response);
    curl_easy_reset(m_curlHandle.get());
    return entries;
}

bool DirectoryCache::isFile(const std::string& path) {
    QMutexLocker locker(&m_mutex);
    size_t pos = path.find_last_of("/");
    std::string directoryPath = path.substr(0, pos);
    std::string fileName = path.substr(pos + 1, path.size());
    if (isPathInCache(directoryPath)) {
        return false;
    }
    const auto& entries = m_cache.at(directoryPath);
    auto it = std::find_if(entries.begin(), entries.end(), [&](const DirectoryEntry& entry) {
        return entry.m_name == fileName; });

    
    return it != entries.end() && it->m_isFile;
}

const uint64_t DirectoryCache::getTotalBytes(const std::string& path, const std::string& fileName) {
   // QMutexLocker locker(&m_mutex);
    if (!isPathInCache(path)) {
        return 0;
    }
    auto& entries = m_cache.find(path)->second;
    auto it = std::find_if(entries.begin(), entries.end(), [&fileName](const DirectoryEntry& entry) {
        return entry.m_name == fileName;
        });
    return it != entries.end() ? it->m_totalBytes : 0;
}

bool DirectoryCache::getCachedDirectory(const std::string& path, std::vector<DirectoryEntry>& entries)  {
    QMutexLocker locker(&m_mutex);
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        entries = it->second;
        return true;
    }
    return false;
}

bool DirectoryCache::isRegularFile(const std::string& filePath) {
    QMutexLocker locker(&m_mutex);
    return !isPathInCache(filePath + "/");
}

void DirectoryCache::refreshDirectory(const std::string& path) {
    std::vector<DirectoryEntry> entries = listDirectory(path);
     m_cache[path] = entries;
    {
       // QMutexLocker locker(&m_mutex);
    }
    emit onDirectoryUpdated(path);
}

void DirectoryCache::reset() {
    m_curlHandle.reset();
    m_curlCode = 0;
    m_baseUrl = "";
    m_cache.clear();
    m_initialized = false;
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
        std::string permissions, hardLinks, owner, group, strSize, month, day, timeOrYear, name;

        lineStream >> permissions >> hardLinks >> owner >> group >> strSize >> month >> day >> timeOrYear;
        std::getline(lineStream, name);

        name = name.substr(name.find_first_not_of(' '));

        if (day.size() == 1) {
            day = '0' + day;
        }

        DirectoryEntry entry;
        entry.m_isDirectory = permissions[0] == 'd';
        entry.m_isSymLink = permissions[0] == 'l';
        entry.m_isFile = permissions[0] == '-';
        entry.m_totalBytes = std::stoul(strSize);
        entry.m_name = name;
        entry.m_owner = owner;
        entry.m_permissions = permissions;
        entry.m_tLastModified = parseDateFromLs(month, day, timeOrYear);

        entries.push_back(entry);
    }
}
