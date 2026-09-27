#pragma once

#include <filesystem>
#include <string>

namespace athena::activation {

void activate(
    const std::filesystem::path& store_directory
);

bool target_is_free(
    const std::filesystem::path& target
);

}
