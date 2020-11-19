# Set the minimum version of CMake that can be used
cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 14)
# Set the project name
project(piapf)

# set a project version
set (piapf_VERSION_MAJOR 0)
set (piapf_VERSION_MINOR 2)
set (piapf_VERSION_PATCH 2)
set (piapf_VERSION "${piapf_VERSION_MAJOR}.${piapf_VERSION_MINOR}.${piapf_VERSION_PATCH}")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Included the conan build information
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    src/main.cpp
    src/uuid.cpp
    src/net_tools.cpp
    src/configuration.cpp
    src/base64.cpp
    src/pia_client.cpp
    src/shell.cpp
    src/subprocess.cpp
)

# Add an executable with the above sources
add_executable(${PROJECT_NAME} ${SOURCES})
# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
# link against the sqlite3 target supplied by conan
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        CONAN_PKG::sqlite3
		CONAN_PKG::cpprestsdk
		Threads::Threads
)


############################################################
# Install
############################################################
# Directory with systemd unit files
set (SYSTEMD_UNIT_DIR "/usr/lib/systemd/system/")
# Binaries
install (TARGETS ${PROJECT_NAME}
    DESTINATION bin)


# Config
install (FILES config
    DESTINATION etc/${PROJECT_NAME})
install (FILES piapf.service
    DESTINATION ${SYSTEMD_UNIT_DIR})

############################################################
# Create DEB
############################################################

# Tell CPack to generate a .deb package
set(CPACK_GENERATOR "DEB")

# Set a Package Maintainer.
# This is required
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Brunino criniti")

# Set a Package Version
set(CPACK_PACKAGE_VERSION ${deb_example_VERSION})

# Include CPack
include(CPack)