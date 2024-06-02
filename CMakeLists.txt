cmake_minimum_required(VERSION 3.19)

# Set project name and version
project(SftpClient VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_PREFIX_PATH "/home/irogulj/Qt/5.15.2/gcc_64")

# Find the Qt5 package
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Add the executable
add_executable(SftpClient main.cpp)

# Link the Qt5 Widgets library
target_link_libraries(SftpClient Qt5::Widgets)

# Add custom target to run the setup_display.sh script
add_custom_target(setup_display
  COMMAND bash -c "source ~/setup_display.sh"
)

# Add custom target to run the application with the display setup
add_custom_target(run_with_display
  COMMAND bash -c "source ~/setup_display.sh && ./SftpClient"
  DEPENDS SftpClient
)

# Ensure that run_with_display depends on setup_display
add_dependencies(run_with_display setup_display)