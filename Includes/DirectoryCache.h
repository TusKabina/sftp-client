#ifndef SFTP_INTERFACE_DIRECTORYCACHE_H
#define SFTP_INTERFACE_DIRECTORYCACHE_H

#include "HandleDeleter.h"
#include <string>
#include <vector>
#include <map>

struct DirectoryEntry {
    std::string m_name;
    uint64_t m_totalBytes;
    bool m_forbidden = false;
    bool m_isDirectory = false;
    bool m_isSymLink = false;
};

class DirectoryCache {
private:
    CurlUniquePtr m_curlHandle;
    int m_curlCode;
    std::string m_baseUrl;
    std::map<std::string, std::vector<DirectoryEntry>> m_cache;
    bool m_initialized;

public:
    DirectoryCache() {}

    [[nodiscard]] bool initialize(const std::string& url, const std::string& user, std::string& password);
    [[nodiscard]] bool isPathInCache(const std::string& path) const { return m_cache.find(path) != m_cache.end(); }
    [[nodiscard]] bool isInitialized() { return m_initialized; }
    [[nodiscard]] const std::map<std::string, std::vector<DirectoryEntry>>& getCache() const { return m_cache; }
    bool getCachedDirectory(const std::string& path, std::vector<DirectoryEntry>& entries) const;
    bool isRegularFile(const std::string& filePath);

    void prefetchDirectories(const std::string& path, int depth);
    void refreshDirectory(const std::string& path);

    ~DirectoryCache() {}

private:
    std::vector<DirectoryEntry> listDirectory(const std::string& path);
    static size_t writeCallback(void* buffer, size_t size, size_t nmemb, std::string& data);
    static size_t dummyWriteCallback(void* buffer, size_t size, size_t nmemb, void* data) { return size * nmemb; }
    void parseResponse(std::vector<DirectoryEntry>& entries, const std::string& response);

};

#endif // SFTP_INTERFACE_DIRECTORYCACHE_H
