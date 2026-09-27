#pragma once

#include <filesystem>
#include <string>

namespace athena::store {

/*
 * Install a staged package into the Athena package store.
 *
 * The package is copied from the staging directory into a dedicated
 * store directory and its metadata is recorded alongside the files.
 *
 * The current store uses:
 *
 *     <package-name>-<package-version>
 *
 * as the directory name.
 *
 * This naming scheme is temporary. The long-term design is a
 * hash-based store inspired by functional package managers.
 */
std::filesystem::path install(
    const std::filesystem::path& staging_directory,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& source,
    const std::string& sha256,
    const std::string& build_system
);

/*
 * Find an installed package by name.
 *
 * The current implementation searches the store for a directory
 * whose name starts with "<package-name>-".
 *
 * The returned path points to the installed package tree.
 */
std::filesystem::path find(
    const std::string& package_name
);

}
