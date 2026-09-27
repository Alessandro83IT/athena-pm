#include "store.hpp"

#include "../paths/paths.hpp"
#include "metadata.hpp"

#include <filesystem>
#include <stdexcept>

namespace athena::store {

/*
 * Install a staged package into the Athena package store.
 *
 * The package is copied from the staging tree into a dedicated
 * directory under /var/lib/athena/store.
 *
 * The current implementation uses the package name and version
 * as the store directory name. A future implementation will use
 * content-derived hashes to make store paths immutable and
 * collision-resistant.
 */
std::filesystem::path install(
    const std::filesystem::path& staging_directory,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& source,
    const std::string& sha256,
    const std::string& build_system
)
{
    /*
     * Construct the destination path for the package.
     *
     * Example:
     *
     *     /var/lib/athena/store/hello-2.12
     */
    const std::filesystem::path destination =
        athena::paths::store() /
        (package_name + "-" + package_version);

    /*
     * Athena currently treats an existing store entry as a duplicate
     * installation rather than replacing it.
     */
    if (std::filesystem::exists(destination)) {
        throw std::runtime_error(
            "Il pacchetto è già presente nello store: " +
            destination.string()
        );
    }

    /*
     * The staging directory contains a DESTDIR-style installation.
     *
     * For a package configured with:
     *
     *     --prefix=/usr
     *
     * the resulting files are expected under:
     *
     *     <staging>/usr/
     */
    const std::filesystem::path installed_root =
        staging_directory / "usr";

    if (!std::filesystem::is_directory(installed_root)) {
        throw std::runtime_error(
            "Directory di installazione non trovata: " +
            installed_root.string()
        );
    }

    /*
     * Create the package's store directory before copying its files.
     */
    std::filesystem::create_directories(destination);

    /*
     * Copy the contents of the staged /usr directory into the store.
     *
     * The current implementation therefore produces a package tree
     * such as:
     *
     *     store/hello-2.12/
     *       ├── bin/
     *       ├── lib/
     *       └── ...
     */
    for (const auto& entry :
         std::filesystem::directory_iterator(installed_root)) {

        std::filesystem::copy(
            entry.path(),
            destination / entry.path().filename(),
            std::filesystem::copy_options::recursive
        );
    }

    /*
     * Record the information needed to identify and inspect the
     * installed package.
     *
     * Metadata is stored separately from the package files under
     * the .athena directory.
     */
    const Metadata metadata{
        package_name,
        package_version,
        source,
        sha256,
        build_system
    };

    write_metadata(
        destination,
        metadata
    );

    return destination;
}

/*
 * Find an installed package by name.
 *
 * The current store layout allows multiple versions to exist,
 * although the current search returns the first matching entry.
 *
 * Future generation and dependency-management logic will require
 * a more precise package database and version-selection mechanism.
 */
std::filesystem::path find(
    const std::string& package_name
)
{
    const std::filesystem::path store_directory =
        athena::paths::store();

    /*
     * The store must exist before it can be searched.
     */
    if (!std::filesystem::exists(store_directory)) {
        throw std::runtime_error(
            "Store non trovato: " +
            store_directory.string()
        );
    }

    /*
     * Search the store for directories whose name starts with
     * "<package-name>-".
     */
    for (const auto& entry :
         std::filesystem::directory_iterator(store_directory)) {

        if (!entry.is_directory()) {
            continue;
        }

        const std::string expected_prefix =
            package_name + "-";

        const std::string directory_name =
            entry.path().filename().string();

        if (directory_name.rfind(expected_prefix, 0) != 0) {
            continue;
        }

        /*
         * A directory is considered a valid Athena package entry
         * only when it contains its metadata file.
         */
        const std::filesystem::path metadata_file =
            entry.path() /
            ".athena" /
            "metadata.toml";

        if (std::filesystem::exists(metadata_file)) {
            return entry.path();
        }
    }

    throw std::runtime_error(
        "Pacchetto non installato: " +
        package_name
    );
}

}
