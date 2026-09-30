#include "resolver.hpp"

#include "../version/version.hpp"

#include <stdexcept>
#include <string>
#include <unordered_set>


namespace athena::dependency {

namespace {

using Package = athena::package::Package;

const Package* find_package(
    const std::string& name,
    const std::vector<Package>& available
)
{
    const Package* best_match = nullptr;

    for (const auto& package : available) {

        if (package.name != name) {
            continue;
        }

        if (best_match == nullptr ||
            athena::version::greater(
                package.version,
                best_match->version
            )) {

            best_match = &package;
        }
    }

    return best_match;
}

void resolve_recursive(
    const Package& package,
    const std::vector<Package>& available,
    std::vector<Package>& result,
    std::unordered_set<std::string>& resolved,
    std::unordered_set<std::string>& resolving
)
{
    /*
     * The package is already completely resolved.
     */
    if (resolved.contains(package.name)) {
        return;
    }

    /*
     * The package is currently being resolved.
     *
     * Encountering it again means that we found a dependency cycle.
     */
    if (resolving.contains(package.name)) {

        throw std::runtime_error(
            "Ciclo di dipendenze rilevato: " +
            package.name
        );
    }

    resolving.insert(package.name);

    /*
     * Resolve all dependencies before adding the package itself.
     *
     * This produces a dependency-first ordering.
     */
    for (const auto& dependency_name : package.dependencies) {

        const Package* dependency =
            find_package(
                dependency_name,
                available
            );

        if (dependency == nullptr) {

            throw std::runtime_error(
                "Dipendenza non trovata: " +
                dependency_name +
                " (richiesta da " +
                package.name +
                ")"
            );
        }

        resolve_recursive(
            *dependency,
            available,
            result,
            resolved,
            resolving
        );
    }

    resolving.erase(package.name);
    resolved.insert(package.name);

    result.push_back(package);
}

}

std::vector<athena::package::Package> resolve(
    const athena::package::Package& root,
    const std::vector<athena::package::Package>& available
)
{
    std::vector<Package> result;

    std::unordered_set<std::string> resolved;
    std::unordered_set<std::string> resolving;

    resolve_recursive(
        root,
        available,
        result,
        resolved,
        resolving
    );

    return result;
}

}
