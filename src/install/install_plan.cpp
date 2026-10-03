#include "install_plan.hpp"

namespace athena::install {

void InstallPlan::add(
    const athena::package::Package& package
)
{
    /*
     * Preserve the order produced by the dependency resolver.
     *
     * Duplicate detection is intentionally not performed here:
     * dependency resolution is responsible for producing a coherent
     * package set.
     */
    packages_.push_back(package);
}

const std::vector<athena::package::Package>&
InstallPlan::packages() const
{
    return packages_;
}

bool InstallPlan::empty() const
{
    return packages_.empty();
}

std::size_t InstallPlan::size() const
{
    return packages_.size();
}

}
