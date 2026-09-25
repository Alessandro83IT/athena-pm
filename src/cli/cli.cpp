#include "cli.hpp"
#include "command_line.hpp"

#include <iostream>

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
    const CommandLine command_line = parse(argc, argv);

    if (command_line.command == "--version" ||
        command_line.command == "-V") {
        std::cout << "Athena Package Manager 0.1.0\n";
        return 0;
    }

    if (command_line.command == "--help" ||
        command_line.command == "-h" ||
        command_line.command.empty()) {
        print_help();
        return 0;
    }

    if (command_line.command == "install") {
        std::cout << "Install command\n";

        if (!command_line.arguments.empty()) {
            std::cout << "Package: " << command_line.arguments[0] << '\n';
        }

        return 0;
    }

    if (command_line.command == "remove") {
        std::cout << "Remove command\n";

        if (!command_line.arguments.empty()) {
            std::cout << "Package: " << command_line.arguments[0] << '\n';
        }

        return 0;
    }

    if (command_line.command == "list") {
        std::cout << "List command\n";
        return 0;
    }

    if (command_line.command == "info") {
        std::cout << "Info command\n";

        if (!command_line.arguments.empty()) {
            std::cout << "Package: " << command_line.arguments[0] << '\n';
        }

        return 0;
    }

    std::cout << "Unknown command: " << command_line.command << '\n';
    return 1;
}

}
