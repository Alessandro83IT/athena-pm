#pragma once

#include <filesystem>

/*
 * Common interface for all Athena package build systems.
 *
 * Every build backend must implement the two operations required
 * by the package installation pipeline:
 *
 *     1. build   - compile the package source;
 *     2. install - install the compiled package into a staging tree.
 *
 * The backend receives filesystem paths but does not know anything
 * about the higher-level package-management workflow.
 */
namespace athena::build {

/*
 * Abstract interface implemented by each build-system backend.
 *
 * Examples of concrete implementations will be:
 *
 *     AutotoolsBackend
 *     CMakeBackend
 *     MesonBackend
 *     MakeBackend
 *
 * Using an interface keeps the build dispatcher independent from
 * the implementation details of each build system.
 */
class BuildBackend {

public:

    /*
     * Virtual destructor required for safe destruction through
     * a pointer or reference to the base class.
     */
    virtual ~BuildBackend() = default;

    /*
     * Build the package contained in source_directory.
     */
    virtual void build(
        const std::filesystem::path& source_directory
    ) = 0;

    /*
     * Install the previously built package into staging_directory.
     */
    virtual void install(
        const std::filesystem::path& source_directory,
        const std::filesystem::path& staging_directory
    ) = 0;
};

}
