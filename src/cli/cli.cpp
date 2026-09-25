#include "cli.hpp"

#include <iostream>
#include <string_view>

namespace athena::cli {

void print_help()
{
    std::cout
        << "Athena Package Manager 0.1.0\n\n"
        << "Usage:\n"
        << "  athena <command> [options]\n\n"
        << "Commands:\n"
        << "  install     Install a package\n"
        << "  remove      Remove a package\n"
        << "  list        List installed packages\n"
        << "  info        Show package information\n\n"
        << "Options:\n"
        << "  -h, --help      Show this help message\n"
        << "  -V, --version   Show version information\n";
}

int run(int argc, char* argv[])
{
    if (argc >= 2) {
        const std::string_view command{argv[1]};

        if (command == "--version" || command == "-V") {
            std::cout << "Athena Package Manager 0.1.0\n";
            return 0;
        }

        if (command == "--help" || command == "-h") {
            print_help();
            return 0;
        }
    }

    print_help();
    return 0;
}

}

