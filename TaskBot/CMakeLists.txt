cmake_minimum_required(VERSION 3.10)
project(TaskBot)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/task_manager.cpp
    src/file_operations.cpp
    src/system_monitor.cpp
    src/command_executor.cpp
    src/logger.cpp
    src/config_manager.cpp
    src/scheduler.cpp
    src/utils.cpp
    src/input_simulator.cpp
    src/web_search.cpp
    src/autopilot_manager.cpp
)

# Create executable
add_executable(taskbot ${SOURCES})

# Find required packages
find_package(X11 REQUIRED)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)

# Find jsoncpp using pkg-config
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Include directories
include_directories(${X11_INCLUDE_DIR})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})

# Link libraries
target_link_libraries(taskbot 
    pthread 
    stdc++fs
    ${X11_LIBRARIES}
    ${X11_Xtst_LIB}
    ${CURL_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

# Installation
install(TARGETS taskbot DESTINATION bin)
install(FILES include/task_bot.h DESTINATION include)

# Create a static library version
add_library(taskbot_lib STATIC
    src/task_manager.cpp
    src/file_operations.cpp
    src/system_monitor.cpp
    src/command_executor.cpp
    src/logger.cpp
    src/config_manager.cpp
    src/scheduler.cpp
    src/utils.cpp
    src/input_simulator.cpp
    src/web_search.cpp
    src/autopilot_manager.cpp
)

target_link_libraries(taskbot_lib 
    pthread 
    stdc++fs
    ${X11_LIBRARIES}
    ${X11_Xtst_LIB}
    ${CURL_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

# Install library
install(TARGETS taskbot_lib DESTINATION lib)

# Package configuration
include(CPack)
set(CPACK_PACKAGE_NAME "TaskBot")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A self-sustained automation bot framework")
set(CPACK_PACKAGE_VENDOR "TaskBot Project")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_GENERATOR "TGZ;DEB;RPM")