#pragma once

#include <filesystem>
#include "install_plan.hpp"

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
    const std::filesystem::path& package_file,
    const std::filesystem::path& target_root
);

/*
 * Execute an already-resolved InstallPlan.
 *
 * The dependency resolver decides which packages are required and
 * preserves their dependency-before-dependent ordering. The installer
 * only executes that plan and does not perform dependency resolution.
 */
void install_plan(
    const InstallPlan& plan,
    const std::filesystem::path& target_root
);

}
