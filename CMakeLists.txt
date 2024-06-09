cmake_minimum_required(VERSION 3.19)

# Set project name and version
project(SftpClient VERSION 1.0)
# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_PREFIX_PATH "/home/irogulj/Qt/5.15.2/gcc_64")

include_directories(Includes)

# Globbing header and source files
file(GLOB HEADER_FILES "Includes/*.h")
file(GLOB SOURCE_FILES "Sources/*.cpp")


# Find the Qt5 package
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(CURL REQUIRED)

# Add the executable
add_executable(SftpClient main.cpp 
        moc/moc_mainGui.cpp
        Includes/mainGui.h 
        Includes/HandleDeleter.h
        Includes/TransferJob.h
        Includes/TransferManager.h
        Includes/TransferFile.h
        Includes/TransferStatus.h
        Includes/UIDGenerator.h  
        Includes/DirectoryCache.h 
        Includes/TransferHandle.h
        Sources/mainGui.cpp 
        Sources/TransferStatus.cpp
        Sources/TransferJob.cpp
        Sources/TransferManager.cpp
        Sources/DirectoryCache.cpp
        Sources/TransferHandle.cpp)

# Link the Qt5 Widgets library
target_link_libraries(SftpClient Qt5::Widgets)
target_link_libraries(SftpClient ${CURL_LIBRARIES})
