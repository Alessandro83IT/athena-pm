#pragma once

#include <string>
#include <vector>

namespace athena::package {

/*
 * Represents the definition of a package.
 *
 * A Package is created by the package loader from a package.toml
 * file and is then passed through the installation pipeline.
 *
 * The structure currently contains the basic information required
 * to download, verify and build a package.
 */
struct Package {

    // Package name, for example: "hello".
    std::string name;

    // Package version, for example: "2.12".
    std::string version;

    // Human-readable package description.
    std::string description;

    // URL of the source archive.
    std::string source;

    // Expected SHA-256 checksum of the source archive.
    std::string sha256;

    /*
     * Build system used to compile the package.
     *
     * Examples:
     *   "autotools"
     *   "cmake"
     *   "meson"
     *
     * The actual build system implementation is handled by
     * the build module.
     */
    std::string build_system;

    /*
     * Names of packages required by this package.
     *
     * Version constraints will be added later.
     */
    std::vector<std::string> dependencies;
};

/*
 * Print the basic information contained in a Package.
 *
 * This function is mainly useful for CLI output and debugging.
 */
void print_info(const Package& package);

}
