#ifndef SFTP_INTERFACE_DIRECTORYENTRY_H
#define SFTP_INTERFACE_DIRECTORYENTRY_H
#include <string>

struct DirectoryEntry {
    std::string m_name;
    std::string m_type;
    std::string m_lastModified;
    std::string m_owner;
    std::string m_permissions;
    time_t m_tLastModified;
    uint64_t m_totalBytes;
    bool m_forbidden = false;
    bool m_isDirectory = false;
    bool m_isSymLink = false;
    bool m_isFile = false;
    bool m_isHidden = false;
};

#endif // SFTP_INTERFACE_DIRECTORYENTRY_H