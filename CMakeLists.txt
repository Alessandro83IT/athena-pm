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
)

target_include_directories(athena
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
