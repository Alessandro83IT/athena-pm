#pragma once

#include <filesystem>
#include <string>

namespace athena::download {

std::filesystem::path download_file(
    const std::string& url,
    const std::filesystem::path& destination
);

}
