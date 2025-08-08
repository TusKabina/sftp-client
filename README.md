# SFTP Client
SFTP Client is multi-threaded C++ Qt application for managing files over SFTP for Linux operating systems. The application is still in active development with plans to add more features in the future.

![Application Screenshot](docs/screenshot.png)

# Features
Download files from SFTP server  
Upload files to SFTP server  
Move files between directories on the server  
Copy files on the server  
Delete files from the server  
Drag & Drop support  
View local and remote directories in split view  
Directory search on the server  
Logger with multiple log levels  
Rename local file  
Delete local file  

## Requirements
**QT 5.15.2**  
**libcurl**  
**CMake 3.19+**  

## Build on Linux
```bash
git clone <repo_url>
cd sftp-client
export QTDIR=/path/to/Qt/5.15.2./gcc_64
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=QTDIR
cmake --build .
```

```bash
Run:
./SftpClient
```

## Build With Visual Studio
To build with Visual Studio 2022 with cross-platform feature, there is CMakePresets.json that contains hardcoded paths for **QT_DIR5**, and **QT_QPA_PLATFORM_PLUGIN_PATH**. They need to be changed so they point to your correct path.
