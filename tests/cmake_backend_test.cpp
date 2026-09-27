#include "../src/build/cmake_backend.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    const std::filesystem::path source_directory =
        "/tmp/athena-cmake-test";

    const std::filesystem::path build_directory =
        "/tmp/athena-cmake-test-build";

    const std::filesystem::path staging_directory =
        "/tmp/athena-cmake-test-staging";

    std::cout << "Testing CMake backend...\n";

    athena::build::CMakeBackend backend;

    backend.build(
        source_directory,
        build_directory
    );

    std::cout << "Build completed.\n";

    backend.install(
        source_directory,
        build_directory,
        staging_directory
    );

    std::cout << "Installation into staging completed.\n";

    return 0;
}
