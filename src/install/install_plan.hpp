#pragma once

#include "../package/package.hpp"

#include <vector>

namespace athena::install {

/*
 * Represents the result of dependency resolution that is ready
 * to be consumed by the installation layer.
 *
 * InstallPlan deliberately contains package definitions rather than
 * performing any installation work. This keeps dependency resolution
 * separate from downloading, building, storing and activating packages.
 */
class InstallPlan {

public:

    /*
     * Add one package to the installation plan.
     *
     * The resolver determines which packages belong to the plan;
     * InstallPlan only stores that result.
     */
    void add(
        const athena::package::Package& package
    );

    /*
     * Return the packages in installation order.
     *
     * The current resolver already produces dependency-before-dependent
     * ordering, so the plan preserves that order without reordering it.
     */
    const std::vector<athena::package::Package>& packages() const;

    /*
     * Return whether the plan contains no packages.
     */
    bool empty() const;

    /*
     * Return the number of packages in the plan.
     */
    std::size_t size() const;

private:

    /*
     * Package definitions selected by the dependency resolver.
     *
     * The vector intentionally preserves insertion order because that
     * order represents the dependency resolution result.
     */
    std::vector<athena::package::Package> packages_;
};

}
