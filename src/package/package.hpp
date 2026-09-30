#pragma once

#include "../dependency/expression.hpp"

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
     * Dependencies required by this package.
     *
     * Dependencies are represented using Athena's universal
     * dependency expression model, allowing package names,
     * capabilities and version constraints to be represented
     * independently of a specific package ecosystem.
     */
    std::vector<athena::dependency::Dependency> dependencies;
};

/*
 * Print the basic information contained in a Package.
 *
 * This function is mainly useful for CLI output and debugging.
 */
void print_info(const Package& package);

}
