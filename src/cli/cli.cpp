#include "cli.hpp"
#include "command_line.hpp"
#include "../package/package.hpp"
#include "../package/package_loader.hpp"
#include "../download/downloader.hpp"
#include "../verify/verifier.hpp"
#include "../install/installer.hpp"
#include "../paths/paths.hpp"
#include "../archive/archive.hpp"
#include "../store/store.hpp"

#include <iostream>
#include <filesystem>

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

    athena::paths::initialize();

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

    if (command_line.command == "paths") {
        std::cout
            << "Root: " << athena::paths::root() << '\n'
            << "Downloads: " << athena::paths::downloads() << '\n'
            << "Sources: " << athena::paths::sources() << '\n'
            << "Build: " << athena::paths::build() << '\n'
            << "Store: " << athena::paths::store() << '\n';

        return 0;
    }

    if (command_line.command == "store-test") {
          const std::filesystem::path staging =
              "/tmp/athena-install-test";
        try {
            const auto installed =
                athena::store::install(
                    staging,
                    "hello",
                    "2.12"
                );

            std::cout
                << "Installazione nello store completata: "
                << installed << '\n';

            return 0;
        }
        catch (const std::exception& error) {
            std::cerr
                << "Errore: " << error.what() << '\n';

            return 1;
        }
    }

    if (command_line.command == "extract-test") {
        const std::filesystem::path archive =
            athena::paths::downloads() /
            "athena-hello-2.12.tar.gz";

        const std::filesystem::path destination =
            "/tmp/athena-extract-test";

        try {
            const auto extracted =
                athena::archive::extract(
                    archive,
                    destination
                );

            std::cout
                << "Estrazione completata: "
                << extracted << '\n';

            return 0;
        }
        catch (const std::exception& error) {
            std::cerr
                << "Errore: " << error.what() << '\n';

            return 1;
        }
    }

    if (command_line.command == "download-test") {
        const std::filesystem::path destination =
            "/tmp/athena-hello-2.12.tar.gz";

        try {
            const auto downloaded =
                athena::download::download_file(
                    "https://ftp.gnu.org/gnu/hello/hello-2.12.tar.gz",
                    destination
                );

            std::cout << "Download completato: "
                      << downloaded << '\n';

            return 0;
        }
        catch (const std::exception& error) {
            std::cerr << "Errore: " << error.what() << '\n';
            return 1;
        }
    }

    if (command_line.command == "install") {
        if (command_line.arguments.empty()) {
            std::cout << "Usage: athena install <package>\n";
            return 1;
        }

        const std::filesystem::path package_file =
            "packages/" + command_line.arguments[0] + "/package.toml";

        try {
            athena::install::install_package(package_file);
            return 0;
        }
        catch (const std::exception& error) {
            std::cerr << "Errore: " << error.what() << '\n';
            return 1;
        }
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
        if (command_line.arguments.empty()) {
            std::cout << "Usage: athena info <package>\n";
            return 1;
        }

        const std::filesystem::path package_file =
            "packages/" + command_line.arguments[0] + "/package.toml";


    try {
        const athena::package::Package package =
            athena::package::load_from_file(package_file);

        athena::package::print_info(package);
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "Errore: " << error.what() << '\n';
        return 1;
    }

    }

    std::cout << "Unknown command: " << command_line.command << '\n';
    return 1;
}

}
