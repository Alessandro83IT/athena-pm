cmake_minimum_required(VERSION 3.20)

project(athena
    VERSION 0.1.0
    DESCRIPTION "Athena Package Manager"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(athena
    src/main.cpp
    src/cli/cli.cpp
    src/cli/command_line.cpp
    src/package/package.cpp
    src/package/package_loader.cpp
    src/download/downloader.cpp
    src/verify/verifier.cpp
    src/install/installer.cpp
    src/paths/paths.cpp
    src/archive/archive.cpp
    src/build/builder.cpp
    src/build/autotools_backend.cpp
    src/build/cmake_backend.cpp
    src/store/store.cpp
    src/store/metadata.cpp
    src/store/manifest.cpp
    src/activation/activation.cpp
)

target_include_directories(athena
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# Standalone test for the CMake build backend.
add_executable(cmake_backend_test
    tests/cmake_backend_test.cpp
    src/build/cmake_backend.cpp
)

target_include_directories(cmake_backend_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/build
)
