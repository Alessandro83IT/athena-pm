#include "../src/package/package_loader.hpp"

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

    assert(
        dependency.expression->kind ==
        athena::dependency::Expression::Kind::Package
    );

    const auto* package_ref =
        dynamic_cast<const athena::dependency::PackageRef*>(
            dependency.expression.get()
        );

    assert(package_ref != nullptr);

    assert(
        package_ref->name ==
        "test-dependency"
    );

    std::cout
        << "Package loader dependency test passed.\n";

    return 0;
}
