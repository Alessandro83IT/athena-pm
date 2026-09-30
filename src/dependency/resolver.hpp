#pragma once

#include "../package/package.hpp"

#include <vector>

namespace athena::dependency {

/*
 * Resolve the dependencies of a package.
 *
 * The resolver receives the package to resolve and a collection
 * of available package definitions.
 *
 * The returned vector contains the packages required by the root
 * package, including the root package itself.
 */
std::vector<athena::package::Package> resolve(
    const athena::package::Package& root,
    const std::vector<athena::package::Package>& available
);

}
