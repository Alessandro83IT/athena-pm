#include "resolver.hpp"

#include "constraint.hpp"
#include "../repository/repository_index.hpp"
#include "../version/version.hpp"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace athena::dependency {

namespace {

using Package = athena::package::Package;

struct NegativeDependency {
    std::string name;
    VersionConstraint constraint;
};

struct ResolutionState {
    std::vector<Package> result;
    std::unordered_set<std::string> resolved;
    std::unordered_set<std::string> resolving;
    std::vector<NegativeDependency> negative_dependencies;
};

/*
 * Find the newest available package version satisfying
 * the specified version constraint.
 */
std::optional<Package> find_package(
    const std::string& name,
    const VersionConstraint& constraint,
    const athena::repository::RepositoryIndex& repository
)
{
    std::optional<Package> best_match;

    /*
     * RepositoryIndex provides all versions of the requested package.
     * The resolver remains responsible for applying the version
     * constraint and selecting the newest compatible candidate.
     */
    for (const auto& package : repository.find(name)) {

        if (!satisfies(constraint, package.version)) {
            continue;
        }

        if (!best_match ||
            athena::version::greater(
                package.version,
                best_match->version
            )) {

            best_match = package;
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

bool violates_negative_dependency(
    const Package& package,
    const std::vector<NegativeDependency>& constraints
)
{
    for (const auto& constraint : constraints) {

        if (package.name != constraint.name) {
            continue;
        }

        if (satisfies(constraint.constraint, package.version)) {
            return true;
        }
    }

    return false;
}

void resolve_expression(
    const ExpressionPtr& expression,
    const Package& requiring_package,
    const athena::repository::RepositoryIndex& repository,
    ResolutionState& state
);

void resolve_package_target(
    const DependencyTarget& target,
    const Package& requiring_package,
    const athena::repository::RepositoryIndex& repository,
    ResolutionState& state
)
{
    const auto dependency_package =
        find_package(
            target.name,
            target.constraint,
            repository
        );

    if (!dependency_package) {

        throw std::runtime_error(
            "Dipendenza non trovata o nessuna versione "
            "compatibile disponibile: " +
            target.name +
            " (richiesta da " +
            requiring_package.name +
            ")"
        );
    }

    if (violates_negative_dependency(
            dependency_package.value(),
            state.negative_dependencies
        )) {

        throw std::runtime_error(
            "Dipendenza vietata dal vincolo NOT: " +
            dependency_package->name +
            " (" +
            dependency_package->version +
            ")"
        );
    }

    /*
     * Resolve the selected package recursively.
     *
     * The optional contains a complete Package value, so the resolver
     * can safely keep using it while traversing the dependency graph.
     */
    if (state.resolved.contains(dependency_package->name)) {
        return;
    }

    if (state.resolving.contains(dependency_package->name)) {

        throw std::runtime_error(
            "Ciclo di dipendenze rilevato: " +
            dependency_package->name
        );
    }

    state.resolving.insert(dependency_package->name);

    for (const auto& dependency :
         dependency_package->dependencies) {

        resolve_expression(
            dependency.expression,
            *dependency_package,
            repository,
            state
        );
    }

    state.resolving.erase(dependency_package->name);
    state.resolved.insert(dependency_package->name);

    state.result.push_back(*dependency_package);
}

void resolve_expression(
    const ExpressionPtr& expression,
    const Package& requiring_package,
    const athena::repository::RepositoryIndex& repository,
    ResolutionState& state
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
                repository,
                state
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
                    repository,
                    state
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
                const auto result_size = state.result.size();
                const auto resolved_before = state.resolved;
                const auto resolving_before = state.resolving;
                const auto negative_dependencies_size =
                    state.negative_dependencies.size();

                try {

                    resolve_expression(
                        child,
                        requiring_package,
                        repository,
                        state
                    );

                    return;
                }
                catch (const std::runtime_error& error) {

                    /*
                     * Roll back every change made by the failed
                     * alternative, including packages already resolved,
                     * resolution state, and negative NOT constraints.
                     */
                    state.result.resize(result_size);
                    state.resolved = resolved_before;
                    state.resolving = resolving_before;
                    state.negative_dependencies.resize(
                        negative_dependencies_size
                    );

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

        case Expression::Kind::Not: {

            const auto* not_expression =
                dynamic_cast<const NotExpression*>(
                    expression.get()
                );

            if (not_expression == nullptr ||
                !not_expression->expression) {

                throw std::runtime_error(
                    "Espressione NOT non valida"
                );
            }

            const auto target =
                extract_dependency_target(
                    not_expression->expression
                );

            for (const auto& package : state.result) {

                if (package.name == target.name &&
                    satisfies(
                        target.constraint,
                        package.version
                    )) {

                    throw std::runtime_error(
                        "Vincolo NOT violato dal pacchetto già "
                        "risolto: " +
                        package.name +
                        " (" +
                        package.version +
                        ")"
                    );
                }
            }

            state.negative_dependencies.push_back({
                target.name,
                target.constraint
            });

            return;
        }

        default:
            throw std::runtime_error(
                "Tipo di espressione di dipendenza non "
                "supportato dal resolver"
            );
    }
}

void resolve_recursive(
    const Package& package,
    const athena::repository::RepositoryIndex& repository,
    ResolutionState& state
)
{
    if (state.resolved.contains(package.name)) {
        return;
    }

    if (state.resolving.contains(package.name)) {

        throw std::runtime_error(
            "Ciclo di dipendenze rilevato: " +
            package.name
        );
    }

    state.resolving.insert(package.name);

    for (const auto& dependency : package.dependencies) {

        resolve_expression(
            dependency.expression,
            package,
            repository,
            state
        );
    }

    state.resolving.erase(package.name);
    state.resolved.insert(package.name);

    state.result.push_back(package);
}

}

std::vector<athena::package::Package> resolve(
    const athena::package::Package& root,
    const athena::repository::Repository& repository
)
{
    /*
     * Repository is the public abstraction used by callers. The resolver
     * deliberately delegates to its index instead of duplicating package
     * lookup or repository storage logic.
     */
    return resolve(
        root,
        repository.index()
    );
}

std::vector<athena::package::Package> resolve(
    const athena::package::Package& root,
    const athena::repository::RepositoryIndex& repository
)
{
    ResolutionState state;

    resolve_recursive(
        root,
        repository,
        state
    );

    /*
     * The root package is part of the final solution just like every
     * resolved dependency. A NOT constraint introduced while resolving
     * its dependency graph must therefore be checked against the root
     * before the complete solution is accepted.
     */
    if (violates_negative_dependency(
            root,
            state.negative_dependencies
        )) {

        throw std::runtime_error(
            "Pacchetto radice vietato dal vincolo NOT: " +
            root.name +
            " (" +
            root.version +
            ")"
        );
    }

    return state.result;
}

/*
 * Legacy overload.
 *
 * Keep the vector-based API source-compatible while making the
 * RepositoryIndex-based resolver the canonical implementation.
 */
std::vector<athena::package::Package> resolve(
    const athena::package::Package& root,
    const std::vector<athena::package::Package>& available
)
{
    athena::repository::RepositoryIndex repository;

    for (const auto& package : available) {
        repository.add(package);
    }

    return resolve(
        root,
        repository
    );
}

}
