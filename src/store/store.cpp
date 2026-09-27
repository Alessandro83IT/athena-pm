#include "store.hpp"

#include "../paths/paths.hpp"
#include "manifest.hpp"
#include "metadata.hpp"

#include <filesystem>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace athena::store {

/*
 * Install a staged package into the Athena package store.
 *
 * The package is copied from the staging tree into a dedicated
 * directory under /var/lib/athena.
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
     * Keep track of every regular file installed into the package
     * store. Paths are recorded relative to the package root.
     */
    std::vector<ManifestEntry> manifest_entries;

    /*
     * Recursively walk through the staged installation tree.
     *
     * Directories themselves are not recorded in the manifest.
     * Only regular files are recorded because directories can be
     * recreated or removed automatically when their contents change.
     */
    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(installed_root)) {

        if (!entry.is_regular_file()) {
            continue;
        }

        /*
         * Determine the file path relative to the staged /usr root.
         *
         * Example:
         *
         *     /staging/usr/bin/foo
         *
         * becomes:
         *
         *     bin/foo
         */
        const std::filesystem::path relative_path =
            std::filesystem::relative(
                entry.path(),
                installed_root
            );

        /*
         * Copy the file into the package's store tree while
         * preserving its relative directory structure.
         */
        const std::filesystem::path destination_file =
            destination / relative_path;

        std::filesystem::create_directories(
            destination_file.parent_path()
        );

        std::filesystem::copy_file(
            entry.path(),
            destination_file
        );

        /*
         * Record the relative path in the package manifest.
         */
        manifest_entries.push_back(
            ManifestEntry{
                relative_path.generic_string()
            }
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

    /*
     * Store the list of files belonging to this package.
     *
     * This manifest will later allow the remove command to determine
     * exactly which files belong to the package.
     */
    write_manifest(
        destination,
        manifest_entries
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

/*
 * List all installed package store entries.
 *
 * Only directories containing Athena metadata are considered
 * installed packages.
 */
std::vector<std::filesystem::path> list()
{
    const std::filesystem::path store_directory =
        athena::paths::store();

    if (!std::filesystem::exists(store_directory)) {
        throw std::runtime_error(
            "Store non trovato: " +
            store_directory.string()
        );
    }

    std::vector<std::filesystem::path> packages;

    for (const auto& entry :
         std::filesystem::directory_iterator(store_directory)) {

        if (!entry.is_directory()) {
            continue;
        }

        const std::filesystem::path metadata_file =
            entry.path() /
            ".athena" /
            "metadata.toml";

        if (!std::filesystem::exists(metadata_file)) {
            continue;
        }

        packages.push_back(entry.path());
    }

    /*
     * Keep the output deterministic.
     *
     * This is important because filesystem directory iteration
     * does not guarantee a particular order.
     */
    std::sort(
        packages.begin(),
        packages.end()
    );

    return packages;
}

}
