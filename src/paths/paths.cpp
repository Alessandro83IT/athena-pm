#include "paths.hpp"

namespace athena::paths {

/*
 * Root directory of the Athena-PM filesystem hierarchy.
 *
 * All paths managed by Athena are derived from this directory
 * instead of being hard-coded throughout the codebase.
 */
std::filesystem::path root()
{
    return "/var/lib/athena";
}

/*
 * Directory containing downloaded source archives.
 */
std::filesystem::path downloads()
{
    return root() / "downloads";
}

/*
 * Directory containing extracted package source trees.
 */
std::filesystem::path sources()
{
    return root() / "sources";
}

/*
 * Directory used for build output and temporary staging trees.
 */
std::filesystem::path build()
{
    return root() / "build";
}

/*
 * Directory containing the installed package store.
 *
 * The store is one of the central components of Athena-PM and will
 * later evolve toward a hash-based, immutable package store.
 */
std::filesystem::path store()
{
    return root() / "store";
}

/*
 * Create the complete Athena-PM directory hierarchy.
 *
 * create_directories() is safe when a directory already exists,
 * so initialization can be called repeatedly.
 */
void initialize()
{
    std::filesystem::create_directories(downloads());
    std::filesystem::create_directories(sources());
    std::filesystem::create_directories(build());
    std::filesystem::create_directories(store());
}

}
