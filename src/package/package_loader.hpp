#pragma once

#include "package.hpp"

#include <filesystem>

namespace athena::package {

Package load_from_file(const std::filesystem::path& path);

}
