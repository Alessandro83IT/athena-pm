#pragma once

#include <filesystem>

namespace athena::install {

void install_package(
    const std::filesystem::path& package_file
);

}
