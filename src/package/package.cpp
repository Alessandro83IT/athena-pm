#include "package.hpp"

#include <iostream>

namespace athena::package {

void print_info(const Package& package)
{
    std::cout
        << "Name: " << package.name << '\n'
        << "Version: " << package.version << '\n'
        << "Description: " << package.description << '\n'
        << "Source: " << package.source << '\n'
        << "SHA-256: " << package.sha256 << '\n';
}

}
