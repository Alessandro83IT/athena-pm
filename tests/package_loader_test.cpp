#include "../src/package/package_loader.hpp"
#include "../src/dependency/constraint.hpp"

#include <cassert>
#include <iostream>

int main()
{
    const auto package =
        athena::package::load_from_file(
            "packages/cmake-test/package.toml"
        );

    assert(package.name == "cmake-test");
    assert(package.version == "1.1");

    assert(package.dependencies.size() == 1);

    const auto& dependency =
        package.dependencies[0];

    assert(
        dependency.kind ==
        athena::dependency::DependencyKind::Runtime
    );

    assert(
        dependency.context ==
        athena::dependency::DependencyContext::Target
    );

    assert(dependency.expression != nullptr);

    /*
     * The dependency must now be represented as a
     * version comparison rather than a plain PackageRef.
     */
    assert(
        dependency.expression->kind ==
        athena::dependency::Expression::Kind::VersionComparison
    );

    const auto* comparison =
        dynamic_cast<
            const athena::dependency::VersionComparison*
        >(
            dependency.expression.get()
        );

    assert(comparison != nullptr);
    assert(comparison->target != nullptr);
    assert(comparison->constraint != nullptr);

    /*
     * Verify the package referenced by the comparison.
     */
    const auto* package_ref =
        dynamic_cast<const athena::dependency::PackageRef*>(
            comparison->target.get()
        );

    assert(package_ref != nullptr);

    assert(
        package_ref->name ==
        "test-dependency"
    );

    /*
     * Verify the version constraint.
     */
    assert(
        comparison->constraint->kind ==
        athena::dependency::VersionConstraint::Kind::Comparison
    );

    assert(
        comparison->constraint->comparison_value.op ==
        athena::dependency::ComparisonOperator::GreaterEqual
    );

    assert(
        comparison->constraint->comparison_value.version ==
        "1.2"
    );

    std::cout
        << "Package loader dependency test passed.\n";

    return 0;
}
