#pragma once

#include "../package/package.hpp"
#include "../repository/repository.hpp"
#include "../repository/repository_index.hpp"

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
/*
 * Resolve dependencies using a repository index.
 *
 * The repository is responsible for providing all available versions
 * of a package; the resolver is responsible for selecting a compatible
 * solution.
 */
std::vector<athena::package::Package> resolve(
    const athena::package::Package& root,
    const athena::repository::RepositoryIndex& repository
);

/*
 * Resolve dependencies using a high-level Repository.
 *
 * Repository owns package loading and repository indexing; the resolver
 * only consumes its index when selecting compatible package versions.
 */
std::vector<athena::package::Package> resolve(
    const athena::package::Package& root,
    const athena::repository::Repository& repository
);


/*
 * Compatibility overload for callers that still provide a raw package
 * collection. The implementation converts the collection into a
 * RepositoryIndex and delegates to the repository-based resolver.
 */
std::vector<athena::package::Package> resolve(
    const athena::package::Package& root,
    const std::vector<athena::package::Package>& available
);

}
