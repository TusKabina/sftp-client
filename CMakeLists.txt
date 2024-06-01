cmake_minimum_required(VERSION 3.25)

# Set project name and version
project(SftpClient VERSION 1.0)

# Find the Qt5 package
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Add the executable
add_executable(SftpClient main.cpp)

# Link the Qt5 Widgets library
target_link_libraries(SftpClient Qt5::Widgets)