cmake_minimum_required(VERSION 3.19)

include(${CMAKE_SOURCE_DIR}/Version.cmake)
project(SftpClient VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/Includes)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto -fomit-frame-pointer")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Concurrent)
find_package(CURL REQUIRED)

file(GLOB_RECURSE HEADER_FILES "Includes/*.h")
file(GLOB_RECURSE SOURCE_FILES "Sources/*.cpp")

add_executable(SftpClient
    ${SOURCE_FILES}
    ${HEADER_FILES}
    main.cpp
)

target_link_libraries(SftpClient PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Concurrent
    ${CURL_LIBRARIES}
)

qt5_add_resources(RESOURCE_FILES Resources/Resources.qrc)
target_sources(SftpClient PRIVATE ${RESOURCE_FILES})