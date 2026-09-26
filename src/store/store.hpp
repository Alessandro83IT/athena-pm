#pragma once

#include <filesystem>
#include <string>

namespace athena::store {

std::filesystem::path install(
    const std::filesystem::path& staging_directory,
    const std::string& package_name,
    const std::string& package_version
);

}
