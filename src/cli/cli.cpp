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
#include "../store/metadata.hpp"
#include "../activation/activation.hpp"

#include <iostream>
#include <filesystem>

namespace athena::cli {

/*
 * Print the general command-line help for Athena.
 *
 * This function contains only presentation logic. It does not
 * execute any package-management operation.
 */
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

/*
 * Run the Athena command-line interface.
 *
 * This function is the main dispatcher of the application.
 *
 * Its responsibility is to translate a parsed command into a call
 * to the appropriate Athena subsystem. The actual package-management
 * work is delegated to specialized modules such as install, store,
 * archive and download.
 */
int run(int argc, char* argv[])
{
    /*
     * Convert the raw argc/argv input into Athena's internal
     * CommandLine representation.
     */
    const CommandLine command_line = parse(argc, argv);

    /*
     * Initialize Athena's filesystem hierarchy before any command
     * attempts to access directories under /var/lib/athena.
     */
    athena::paths::initialize();

    /*
     * Handle the global version option.
     */
    if (command_line.command == "--version" ||
        command_line.command == "-V") {

        std::cout << "Athena Package Manager 0.1.0\n";
        return 0;
    }

    /*
     * Display help when explicitly requested or when the user
     * starts Athena without specifying a command.
     */
    if (command_line.command == "--help" ||
        command_line.command == "-h" ||
        command_line.command.empty()) {

        print_help();
        return 0;
    }

    /*
     * Display the filesystem hierarchy currently used by Athena.
     *
     * This is currently a diagnostic command and is useful while
     * developing the package store and build pipeline.
     */
    if (command_line.command == "paths") {

        std::cout
            << "Root: " << athena::paths::root() << '\n'
            << "Downloads: " << athena::paths::downloads() << '\n'
            << "Sources: " << athena::paths::sources() << '\n'
            << "Build: " << athena::paths::build() << '\n'
            << "Store: " << athena::paths::store() << '\n';

        return 0;
    }

    /*
     * Temporary extraction test.
     *
     * This command exists only during development and exercises the
     * archive module independently from the complete installation
     * pipeline.
     */
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

    /*
     * Temporary download test.
     *
     * Like extract-test, this command is intended for development
     * and allows the downloader to be tested independently.
     */
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


    /*
     * Temporary activation test.
     *
     * This command activates a package into a temporary target
     * directory instead of modifying the live system.
     */
    if (command_line.command == "activate-test") {

        if (command_line.arguments.empty()) {
            std::cout
                << "Usage: athena activate-test <store-directory>\n";
            return 1;
        }

        const std::filesystem::path store_directory =
            command_line.arguments[0];

        try {

            athena::activation::activate(
                store_directory,
                "/tmp/athena-activation-test/usr"
            );         

            return 0;
        }
        catch (const std::exception& error) {

            std::cerr
                << "Errore: " << error.what() << '\n';

            return 1;
        }
    }

    /*
     * Temporary deactivation test.
     *
     * This command deactivates a package from a temporary target
     * directory instead of modifying the live system.
     */
    if (command_line.command == "deactivate-test") {

        if (command_line.arguments.empty()) {
            std::cout
                << "Usage: athena deactivate-test <store-directory>\n";
            return 1;
        }

        const std::filesystem::path store_directory =
            command_line.arguments[0];

        try {

            athena::activation::deactivate(
                store_directory,
                "/tmp/athena-activation-test/usr"
            );

            return 0;
        }
        catch (const std::exception& error) {

            std::cerr
                << "Errore: " << error.what() << '\n';

            return 1;
        }
    }

    /*
     * Install a package using its package definition.
     *
     * The CLI only determines which package was requested and
     * constructs the path to package.toml. The installation pipeline
     * itself is implemented by the install module.
     */
    if (command_line.command == "install-test") {

        if (command_line.arguments.empty()) {
            std::cout << "Usage: athena install-test <package>\n";
            return 1;
        }

        const std::filesystem::path package_file =
            "packages/" + command_line.arguments[0] + "/package.toml";

        const std::filesystem::path target_root =
            "/tmp/athena-install-test";

        try {

            athena::install::install_package(
                package_file,
                target_root
            );

            return 0;
        }
        catch (const std::exception& error) {

            std::cerr
                << "Errore: "
                << error.what()
                << '\n';

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

            athena::install::install_package(
                package_file,
                "/tmp/athena-install-test"
            );

            return 0;
        } 
        catch (const std::exception& error) {

             std::cerr << "Errore: " << error.what() << '\n';
             return 1;
        }
    }

     /*
     * Remove a package from the active system.
     *
     * The package remains in the immutable store.
     * Only its activated files are deactivated.
     */
    if (command_line.command == "remove") {

        if (command_line.arguments.empty()) {
            std::cout
                << "Usage: athena remove <package>\n";
            return 1;
        }

        try {

            const std::string package_name =
                command_line.arguments[0];

            const auto store_directory =
                athena::store::find(
                    package_name
                );

            std::cout
                << "Rimozione del pacchetto: "
                << package_name
                << '\n';

            athena::activation::deactivate(
                store_directory,
                "/tmp/athena-install-test/usr"
            );

            std::cout
                << "Rimozione completata.\n";

            return 0;
        }
        catch (const std::exception& error) {

            std::cerr
                << "Errore: "
                << error.what()
                << '\n';

            return 1;
        }
    }

    /*
     * List installed packages.
     *
     * The command queries Athena's package store and displays
     * every installed package version.
     */
    if (command_line.command == "list") {

        const auto packages =
            athena::store::list();

        for (const auto& package_path : packages) {

            const auto metadata =
                athena::store::read_metadata(
                    package_path
                );

            std::cout
                << metadata.name
                << " "
                << metadata.version
                << '\n';
        }

        return 0;
    }

    /*
     * Display metadata for an installed package.
     *
     * All installed store entries are inspected so that multiple
     * installed versions of the same package can be displayed.
     */
    if (command_line.command == "info") {

        if (command_line.arguments.empty()) {
            std::cout << "Usage: athena info <package>\n";
            return 1;
        }

        try {

            const std::string package_name =
                command_line.arguments[0];

            const auto packages =
                athena::store::list();

            bool found = false;

            for (const auto& package_path : packages) {

                const auto metadata =
                    athena::store::read_metadata(
                        package_path
                    );

                if (metadata.name != package_name) {
                    continue;
                }

                found = true;

                std::cout
                    << "Name: " << metadata.name << '\n'
                    << "Version: " << metadata.version << '\n'
                    << "Source: " << metadata.source << '\n'
                    << "SHA-256: " << metadata.sha256 << '\n'
                    << "Build system: " << metadata.build_system << '\n'
                    << "Store: " << package_path << '\n'
                    << '\n';
            }

            if (!found) {
                throw std::runtime_error(
                    "Pacchetto non installato: " +
                    package_name
                );
            }

            return 0;
        }
        catch (const std::exception& error) {

            std::cerr
                << "Errore: " << error.what() << '\n';

            return 1;
        }
    }

    std::cerr
        << "Comando sconosciuto: "
        << command_line.command
        << '\n';

    return 1;
}

}
