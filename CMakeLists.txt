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
)

# Add an executable with the above sources
add_executable(${PROJECT_NAME} ${SOURCES})


# link against the sqlite3 target supplied by conan
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        CONAN_PKG::sqlite3
		CONAN_PKG::cpprestsdk
)