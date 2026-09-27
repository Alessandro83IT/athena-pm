#include "installer.hpp"

#include "../package/package.hpp"
#include "../package/package_loader.hpp"
#include "../download/downloader.hpp"
#include "../verify/verifier.hpp"
#include "../paths/paths.hpp"
#include "../archive/archive.hpp"
#include "../build/builder.hpp"
#include "../store/store.hpp"

#include <iostream>
#include <stdexcept>

namespace athena::install {

/*
 * Install a package from its package definition file.
 *
 * The installer acts as the coordinator of the package-management
 * pipeline. It does not implement downloading, verification,
 * extraction, building or storage itself. Instead, it connects
 * the specialized modules in the correct order.
 *
 * The complete pipeline is:
 *
 *     package.toml
 *         ↓
 *     load package
 *         ↓
 *     download source
 *         ↓
 *     verify checksum
 *         ↓
 *     extract source
 *         ↓
 *     build
 *         ↓
 *     install into staging
 *         ↓
 *     store package
 *
 * Keeping these stages separated makes it possible to replace or
 * extend individual parts of Athena without rewriting the entire
 * installation process.
 */
void install_package(
    const std::filesystem::path& package_file
)
{
    /*
     * Load the package definition and convert the TOML data into
     * Athena's internal Package representation.
     */
    const athena::package::Package package =
        athena::package::load_from_file(package_file);

    std::cout
        << "Installazione di: " << package.name
        << " " << package.version << '\n';

    /*
     * Determine where the source archive will be stored locally.
     *
     * Downloads are kept separate from extracted sources and from
     * the final package store.
     */
    const std::filesystem::path destination =
        athena::paths::downloads() /
        ("athena-" + package.name + "-" + package.version + ".tar.gz");

    std::cout
        << "Download: " << package.source << '\n';

    /*
     * Download the source archive declared by the package
     * definition.
     */
    const auto downloaded =
        athena::download::download_file(
            package.source,
            destination
        );

    std::cout
        << "Download completato: "
        << downloaded << '\n';

    /*
     * Verify that the downloaded archive matches the checksum
     * declared in package.toml.
     *
     * This protects the build pipeline from using a source archive
     * different from the one expected by the package definition.
     */
    std::cout << "Verifica SHA-256...\n";

    const bool valid =
        athena::verify::verify_sha256(
            downloaded,
            package.sha256
        );

    if (!valid) {
        throw std::runtime_error(
            "SHA-256 non corrisponde per: " +
            downloaded.string()
        );
    }

    std::cout << "SHA-256 verificato correttamente.\n";

    /*
     * The source extraction directory is managed by the paths
     * module so that filesystem locations are centralized.
     */
    const std::filesystem::path source_directory =
        athena::paths::sources();

    std::cout
        << "Estrazione in: "
        << source_directory << '\n';

    /*
     * Extract the source archive.
     *
     * The archive module determines the actual top-level source
     * directory and returns its path.
     */
    const auto extracted =
        athena::archive::extract(
            downloaded,
            source_directory
        );

    std::cout
        << "Estrazione completata: "
        << extracted << '\n';

    /*
     * Build the package using the build system declared in
     * package.toml.
     *
     * The installer does not need to know how Autotools, CMake,
     * Meson or other build systems work internally. That decision
     * belongs to the build module.
     */
    std::cout
        << "Compilazione...\n";

    athena::build::build(
        extracted,
        package.build_system
    );

    std::cout
        << "Compilazione completata.\n";

    /*
     * Create a dedicated staging directory for this package.
     *
     * The package is not installed directly into the running
     * system. Instead, make install writes into this temporary
     * filesystem tree.
     */
    const std::filesystem::path staging =
        athena::paths::build() /
        (package.name + "-" + package.version + "-install");

    std::cout
        << "Installazione nello staging: "
        << staging << '\n';

    /*
     * Install the compiled package into the staging tree.
     *
     * The build system determines how the installation is
     * performed, while the staging directory keeps the files
     * isolated from the live filesystem.
     */
    athena::build::install(
        extracted,
        staging,
        package.build_system
    );

    std::cout
        << "Installazione nello staging completata.\n";

    /*
     * Move the staged package into Athena's package store.
     *
     * Along with the package files, the store records metadata
     * such as the package version, source URL, checksum and
     * build system.
     */
    const auto installed =
        athena::store::install(
            staging,
            package.name,
            package.version,
            package.source,
            package.sha256,
            package.build_system
        );

    std::cout
        << "Installazione nello store completata: "
        << installed << '\n';
}

}
