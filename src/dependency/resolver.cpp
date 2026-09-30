#include "resolver.hpp"

#include "constraint.hpp"
#include "../version/version.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>


namespace athena::dependency {

namespace {

using Package = athena::package::Package;

/*
 * Find the newest available package version satisfying
 * the specified version constraint.
 */
const Package* find_package(
    const std::string& name,
    const VersionConstraint& constraint,
    const std::vector<Package>& available
)
{
    const Package* best_match = nullptr;

    for (const auto& package : available) {

        if (package.name != name) {
            continue;
        }

        if (!satisfies(constraint, package.version)) {
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

/*
 * Extract a package name and version constraint from a dependency
 * expression.
 *
 * Currently supported:
 *
 *     PackageRef
 *     VersionComparison(PackageRef, VersionConstraint)
 *
 * A plain PackageRef means Any version.
 */
struct DependencyTarget {
    std::string name;
    VersionConstraint constraint;
};

DependencyTarget extract_dependency_target(
    const ExpressionPtr& expression
)
{
    if (!expression) {
        throw std::runtime_error(
            "Espressione di dipendenza nulla"
        );
    }

    if (expression->kind == Expression::Kind::Package) {

        const auto* package =
            dynamic_cast<const PackageRef*>(
                expression.get()
            );

        if (package == nullptr) {
            throw std::runtime_error(
                "Espressione Package non valida"
            );
        }

        return {
            package->name,
            VersionConstraint::any()
        };
    }

    if (expression->kind ==
        Expression::Kind::VersionComparison) {

        const auto* comparison =
            dynamic_cast<const VersionComparison*>(
                expression.get()
            );

        if (comparison == nullptr ||
            !comparison->target ||
            !comparison->constraint) {

            throw std::runtime_error(
                "Espressione di confronto versione non valida"
            );
        }

        if (comparison->target->kind !=
            Expression::Kind::Package) {

            throw std::runtime_error(
                "Il confronto di versione deve riferirsi "
                "a un pacchetto"
            );
        }

        const auto* package =
            dynamic_cast<const PackageRef*>(
                comparison->target.get()
            );

        if (package == nullptr) {
            throw std::runtime_error(
                "Riferimento al pacchetto non valido"
            );
        }

        return {
            package->name,
            *comparison->constraint
        };
    }

    throw std::runtime_error(
        "Tipo di espressione di dipendenza non ancora supportato"
    );
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
    for (const auto& dependency : package.dependencies) {

        const auto target =
            extract_dependency_target(
                dependency.expression
            );

        const Package* dependency_package =
            find_package(
                target.name,
                target.constraint,
                available
            );

        if (dependency_package == nullptr) {

            throw std::runtime_error(
                "Dipendenza non trovata o nessuna versione "
                "compatibile disponibile: " +
                target.name +
                " (richiesta da " +
                package.name +
                ")"
            );
        }

        resolve_recursive(
            *dependency_package,
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
