#ifndef SFTP_INTERFACE_LOCAL_OPERATIONS_H
#define SFTP_INTERFACE_LOCAL_OPERATIONS_H
#include <string>
#include <Utilities/Logger.h>

class LocalOperations {
public:
    static bool deleteFile(const std::string& path);
    static bool deleteDirectory(const std::string& path);
    static bool createDirectory(const std::string& path);
    static bool copyFile(const std::string& source, const std::string& destination);
    static bool moveFile(const std::string& source, const std::string& destination);
};

#endif // SFTP_INTERFACE_LOCAL_OPERATIONS_H