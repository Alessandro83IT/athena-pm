#pragma once

#include <filesystem>
#include <string>

namespace athena::verify {

bool verify_sha256(
    const std::filesystem::path& file,
    const std::string& expected_hash
);

}
