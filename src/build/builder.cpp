#include "builder.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace athena::build {

/*
 * Build a package from its extracted source directory.
 *
 * The build system is declared in the package definition
 * (package.toml) and is passed to the builder by the installer.
 *
 * At the moment Athena supports only the Autotools build system.
 * Additional build systems will be dispatched from this function.
 */
void build(
    const std::filesystem::path& source_directory,
    const std::string& build_system
)
{
    /*
     * Reject build systems that are not implemented yet.
     *
     * This prevents Athena from silently trying to use the wrong
     * build commands for a package.
     */
    if (build_system != "autotools") {
        throw std::runtime_error(
            "Sistema di build non supportato: " +
            build_system
        );
    }

    /*
     * Autotools build:
     *
     * 1. Enter the extracted source directory.
     * 2. Configure the package to install under /usr.
     * 3. Compile the source code with make.
     */
    const std::string command =
        "cd \"" + source_directory.string() +
        "\" && ./configure --prefix=/usr && make";

    const int result = std::system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(
            "Compilazione fallita in: " +
            source_directory.string()
        );
    }
}

/*
 * Install the already-built package into a staging directory.
 *
 * Athena does not install the files directly into the live system.
 * Instead, the package is installed into a temporary staging tree.
 *
 * The staging tree is later copied into the Athena package store.
 */
void install(
    const std::filesystem::path& source_directory,
    const std::filesystem::path& staging_directory,
    const std::string& build_system
)
{
    /*
     * The same build-system validation used during compilation
     * is performed before the installation phase.
     */
    if (build_system != "autotools") {
        throw std::runtime_error(
            "Sistema di build non supportato: " +
            build_system
        );
    }

    /*
     * Start with an empty staging directory.
     *
     * This prevents files from a previous build from accidentally
     * becoming part of the current package.
     */
    std::filesystem::remove_all(staging_directory);
    std::filesystem::create_directories(staging_directory);

    /*
     * Autotools installation:
     *
     * DESTDIR redirects the installation into the staging tree
     * instead of installing directly into the running system.
     *
     * For example:
     *
     *   /usr/bin/hello
     *
     * becomes:
     *
     *   <staging>/usr/bin/hello
     */
    const std::string command =
        "cd \"" + source_directory.string() +
        "\" && make install DESTDIR=\"" +
        staging_directory.string() + "\"";

    const int result = std::system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(
            "Installazione fallita in: " +
            source_directory.string()
        );
    }
}

}
