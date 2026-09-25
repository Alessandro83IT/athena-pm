#pragma once

#include <string>

namespace athena::package {

struct Package {
    std::string name;
    std::string version;
    std::string description;
    std::string source;
    std::string sha256;
};

void print_info(const Package& package);

}
