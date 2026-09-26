#pragma once

#include <filesystem>
#include <string>

namespace athena::store {

std::filesystem::path install(
    const std::filesystem::path& staging_directory,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& source,
    const std::string& sha256,
    const std::string& build_system
);

std::filesystem::path find(
    const std::string& package_name
);

}
