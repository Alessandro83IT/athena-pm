#pragma once

#include <filesystem>
#include <string>

namespace athena::store {

struct Metadata {
    std::string name;
    std::string version;
    std::string source;
    std::string sha256;
    std::string build_system;
};

void write_metadata(
    const std::filesystem::path& store_directory,
    const Metadata& metadata
);

Metadata read_metadata(
    const std::filesystem::path& store_directory
);

}
