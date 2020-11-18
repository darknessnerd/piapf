# Set the minimum version of CMake that can be used
cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 14)
# Set the project name
project(piapf)

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
)