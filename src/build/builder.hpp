#pragma once

#include <filesystem>

namespace athena::build {

void build(
    const std::filesystem::path& source_directory
);

void install(
    const std::filesystem::path& source_directory,
    const std::filesystem::path& staging_directory
);

}
