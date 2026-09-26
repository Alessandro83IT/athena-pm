#pragma once

#include <filesystem>

namespace athena::paths {

std::filesystem::path root();

std::filesystem::path downloads();

std::filesystem::path sources();

std::filesystem::path build();

std::filesystem::path store();

void initialize();

}
