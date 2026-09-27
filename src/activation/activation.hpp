#pragma once

#include <filesystem>
#include <string>

namespace athena::activation {

void activate(
    const std::filesystem::path& store_directory
);

}
