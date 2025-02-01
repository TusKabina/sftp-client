#ifndef SFTP_INTERFACE_DIRECTORYENTRY_H
#define SFTP_INTERFACE_DIRECTORYENTRY_H
#include <string>

struct DirectoryEntry {
    std::string m_name;
    std::string m_lastModified;
    time_t m_tLastModified;
    std::string m_owner;
    std::string m_permissions;
    uint64_t m_totalBytes;
    bool m_forbidden = false;
    bool m_isDirectory = false;
    bool m_isSymLink = false;
    bool m_isFile = false;
};

#endif // SFTP_INTERFACE_DIRECTORYENTRY_H