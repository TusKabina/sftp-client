#include "LocalOperations.h"
#include <filesystem>

namespace LocalOperations {
	bool deleteFile(const std::string& path) {
		if (std::filesystem::exists(path)) {
			try
			{
				std::filesystem::remove(path);
				return true;
			}
			catch (const std::exception&)
			{
				logger().error() << "Failed to delete file: " << path;
				return false;
			}
		}
		else {
			logger().error() << "File does not exist: " << path;
			return false;
		}
	}

	bool deleteDirectory(const std::string& path) {
		if (std::filesystem::exists(path)) {
			try
			{
				std::filesystem::remove_all(path);
				return true;
			}
			catch (const std::exception&)
			{
				logger().error() << "Failed to delete directory: " << path;
				return false;
			}
		}
		else {
			logger().error() << "Directory does not exist: " << path;
		}
		return false;
	}

	bool createDirectory(const std::string& path) {
		if (std::filesystem::exists(path)) {
			logger().error() << "Directory already exists: " << path;
			return false;
		}
		else {
			try
			{
				std::filesystem::create_directories(path);
				return true;
			}
			catch (const std::exception&)
			{
				logger().error() << "Failed to create directory: " << path;
				return false;
			}
		}
		return false;
	}

	bool copyFile(const std::string& source, const std::string& destination) {
		if (std::filesystem::exists(source)) {
			try
			{
				std::filesystem::copy(source, destination);
				return true;
			}
			catch (const std::exception&)
			{
				logger().error() << "Failed to copy file from: " << source << " to: " << destination;
				return false;
			}
		}
		else {
			logger().error() << "Source file does not exist: " << source;
		}
		return false;
	}

	bool moveFile(const std::string& source, const std::string& destination) {
		if (std::filesystem::exists(source)) {
			try
			{
				std::filesystem::rename(source, destination);
				return true;
			}
			catch (const std::exception&)
			{
				logger().error() << "Failed to move file from: " << source << " to: " << destination;
				return false;
			}
		}
		else {
			logger().error() << "Source file does not exist: " << source;
		}
		return false;
	}
}