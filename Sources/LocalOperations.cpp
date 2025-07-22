#include "LocalOperations.h"
#include <filesystem>

bool LocalOperations::deleteFile(const std::string& path) {
	return false;
}

bool LocalOperations::deleteDirectory(const std::string& path) {
	return false;
}

bool LocalOperations::createDirectory(const std::string& path) {
	return false;
}

bool LocalOperations::copyFile(const std::string& source, const std::string& destination) {
	return false;
}

bool LocalOperations::moveFile(const std::string& source, const std::string& destination) {
	return false;
}