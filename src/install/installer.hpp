#pragma once

#include <filesystem>

namespace athena::install {

/*
 * Install a package described by a package definition file.
 *
 * This function coordinates the complete package installation
 * pipeline:
 *
 *     package definition
 *          ↓
 *        download
 *          ↓
 *        verify
 *          ↓
 *        extract
 *          ↓
 *         build
 *          ↓
 *       staging
 *          ↓
 *         store
 *
 * The package is not installed directly into the live system.
 * Instead, the builder creates a staged filesystem tree which
 * is then transferred into the Athena package store.
 */
void install_package(
    const std::filesystem::path& package_file
);

}
