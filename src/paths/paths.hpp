#pragma once

#include <filesystem>

namespace athena::paths {

/*
 * Return the root directory used by Athena-PM for its local data.
 *
 * The current prototype stores all Athena data under:
 *
 *     /var/lib/athena
 *
 * The exact layout may evolve as the package store and generation
 * system become more sophisticated.
 */
std::filesystem::path root();

/*
 * Directory containing downloaded package archives.
 */
std::filesystem::path downloads();

/*
 * Directory containing extracted package source trees.
 */
std::filesystem::path sources();

/*
 * Directory used for temporary build and staging data.
 */
std::filesystem::path build();

/*
 * Directory containing installed Athena package trees.
 */
std::filesystem::path store();

/*
 * Create the directory structure required by Athena-PM.
 *
 * This function should be called before other modules start using
 * the Athena filesystem hierarchy.
 */
void initialize();

}
