#pragma once

#include <filesystem>
#include <string>

namespace athena::build {

/*
 * Build a package from its extracted source directory.
 *
 * The build system is selected from the package definition.
 * The builder is responsible only for compiling the source;
 * it does not install files into the live system.
 *
 * The resulting build artifacts remain inside the source/build
 * environment until the installation phase.
 */
void build(
    const std::filesystem::path& source_directory,
    const std::string& build_system
);

/*
 * Install a previously built package into a staging directory.
 *
 * The staging directory acts as a temporary filesystem root.
 * This allows Athena to collect the files belonging to the
 * package before placing them in the package store.
 *
 * Files are therefore never installed directly into the running
 * system by this function.
 */
void install(
    const std::filesystem::path& source_directory,
    const std::filesystem::path& staging_directory,
    const std::string& build_system
);

}
