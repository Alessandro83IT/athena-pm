#include "resolver.hpp"

#include "constraint.hpp"
#include "../version/version.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

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
 * Extract a package name and version constraint from a simple
 * dependency expression.
 *
 * Supported:
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
        "Tipo di espressione non semplice: "
        "non può essere convertita in un singolo target"
    );
}

void resolve_expression(
    const ExpressionPtr& expression,
    const Package& requiring_package,
    const std::vector<Package>& available,
    std::vector<Package>& result,
    std::unordered_set<std::string>& resolved,
    std::unordered_set<std::string>& resolving
);

void resolve_package_target(
    const DependencyTarget& target,
    const Package& requiring_package,
    const std::vector<Package>& available,
    std::vector<Package>& result,
    std::unordered_set<std::string>& resolved,
    std::unordered_set<std::string>& resolving
)
{
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
            requiring_package.name +
            ")"
        );
    }

    /*
     * Resolve the selected package recursively.
     */
    if (resolved.contains(dependency_package->name)) {
        return;
    }

    if (resolving.contains(dependency_package->name)) {

        throw std::runtime_error(
            "Ciclo di dipendenze rilevato: " +
            dependency_package->name
        );
    }

    resolving.insert(dependency_package->name);

    for (const auto& dependency :
         dependency_package->dependencies) {

        resolve_expression(
            dependency.expression,
            *dependency_package,
            available,
            result,
            resolved,
            resolving
        );
    }

    resolving.erase(dependency_package->name);
    resolved.insert(dependency_package->name);

    result.push_back(*dependency_package);
}

void resolve_expression(
    const ExpressionPtr& expression,
    const Package& requiring_package,
    const std::vector<Package>& available,
    std::vector<Package>& result,
    std::unordered_set<std::string>& resolved,
    std::unordered_set<std::string>& resolving
)
{
    if (!expression) {
        throw std::runtime_error(
            "Espressione di dipendenza nulla"
        );
    }

    switch (expression->kind) {

        case Expression::Kind::Package:
        case Expression::Kind::VersionComparison: {

            const auto target =
                extract_dependency_target(
                    expression
                );

            resolve_package_target(
                target,
                requiring_package,
                available,
                result,
                resolved,
                resolving
            );

            return;
        }

        case Expression::Kind::And: {

            const auto* and_expression =
                dynamic_cast<const AndExpression*>(
                    expression.get()
                );

            if (and_expression == nullptr) {
                throw std::runtime_error(
                    "Espressione AND non valida"
                );
            }

            for (const auto& child :
                 and_expression->expressions) {

                resolve_expression(
                    child,
                    requiring_package,
                    available,
                    result,
                    resolved,
                    resolving
                );
            }

            return;
        }

        case Expression::Kind::Or: {

            const auto* or_expression =
                dynamic_cast<const OrExpression*>(
                    expression.get()
                );

            if (or_expression == nullptr) {
                throw std::runtime_error(
                    "Espressione OR non valida"
                );
            }

            std::string last_error;

            for (const auto& child :
                 or_expression->expressions) {

                /*
                 * Each OR alternative is attempted transactionally.
                 *
                 * If the alternative fails, all state changes produced
                 * by that attempt are rolled back before trying the
                 * next alternative.
                 */
                const auto result_size = result.size();
                const auto resolved_before = resolved;
                const auto resolving_before = resolving;

                try {

                    resolve_expression(
                        child,
                        requiring_package,
                        available,
                        result,
                        resolved,
                        resolving
                    );

                    return;
                }
                catch (const std::runtime_error& error) {

                    result.resize(result_size);
                    resolved = resolved_before;
                    resolving = resolving_before;

                    last_error = error.what();
                }
            }

            throw std::runtime_error(
                "Nessuna alternativa soddisfacibile per "
                "la dipendenza richiesta da " +
                requiring_package.name +
                (last_error.empty()
                    ? ""
                    : ": " + last_error)
            );
        }

        case Expression::Kind::Not:
            throw std::runtime_error(
                "Espressione NOT non ancora supportata "
                "dal resolver"
            );

        default:
            throw std::runtime_error(
                "Tipo di espressione di dipendenza non "
                "supportato dal resolver"
            );
    }
}

void resolve_recursive(
    const Package& package,
    const std::vector<Package>& available,
    std::vector<Package>& result,
    std::unordered_set<std::string>& resolved,
    std::unordered_set<std::string>& resolving
)
{
    if (resolved.contains(package.name)) {
        return;
    }

    if (resolving.contains(package.name)) {

        throw std::runtime_error(
            "Ciclo di dipendenze rilevato: " +
            package.name
        );
    }

    resolving.insert(package.name);

    for (const auto& dependency : package.dependencies) {

        resolve_expression(
            dependency.expression,
            package,
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
