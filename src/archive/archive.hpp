#pragma once

#include <filesystem>

namespace athena::archive {

std::filesystem::path extract(
    const std::filesystem::path& archive,
    const std::filesystem::path& destination
);

}
