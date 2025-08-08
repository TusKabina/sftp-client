#ifndef SFTP_INTERFACE_LOCAL_OPERATIONS_H
#define SFTP_INTERFACE_LOCAL_OPERATIONS_H
#include <string>
#include <Utilities/Logger.h>


namespace LocalOperations {
	bool deleteFile(const std::string& path);
	bool deleteDirectory(const std::string& path);
	bool createDirectory(const std::string& path);
	bool copyFile(const std::string& source, const std::string& destination);
	bool moveFile(const std::string& source, const std::string& destination);
};

#endif // SFTP_INTERFACE_LOCAL_OPERATIONS_H