#include "../src/dependency/resolver.hpp"
#include "../src/dependency/constraint.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using athena::package::Package;
using namespace athena::dependency;

namespace {

Dependency package_dependency(
    const std::string& name
)
{
    return Dependency{
        std::make_shared<const PackageRef>(name),
        DependencyKind::Runtime,
        DependencyContext::Target
    };
}

Dependency versioned_dependency(
    const std::string& name,
    VersionConstraint constraint
)
{
    auto package =
        std::make_shared<const PackageRef>(name);

    auto version_comparison =
        std::make_shared<const VersionComparison>(
            package,
            std::make_shared<const VersionConstraint>(
                std::move(constraint)
            )
        );

    return Dependency{
        version_comparison,
        DependencyKind::Runtime,
        DependencyContext::Target
    };
}

}

int main()
{
    /*
     * Several versions of zlib are available.
     */
    Package zlib_old{
        "zlib",
        "1.0",
        "Zlib",
        "",
        "",
        "autotools",
        {}
    };

    Package zlib_mid{
        "zlib",
        "1.8",
        "Zlib",
        "",
        "",
        "autotools",
        {}
    };

    Package zlib_new{
        "zlib",
        "2.0",
        "Zlib",
        "",
        "",
        "autotools",
        {}
    };

    /*
     * A package with an unconstrained dependency.
     *
     * The resolver must select the newest version.
     */
    Package libfoo{
        "libfoo",
        "1.0",
        "Libfoo",
        "",
        "",
        "autotools",
        {
            package_dependency("zlib")
        }
    };

    Package app{
        "app",
        "1.0",
        "Test application",
        "",
        "",
        "autotools",
        {
            package_dependency("libfoo")
        }
    };

    const std::vector<Package> available{
        zlib_old,
        zlib_mid,
        zlib_new,
        libfoo
    };

    const auto result =
        athena::dependency::resolve(
            app,
            available
        );

    /*
     * Dependency-first ordering:
     *
     *     zlib 2.0
     *     libfoo 1.0
     *     app 1.0
     */
    assert(result.size() == 3);

    assert(result[0].name == "zlib");
    assert(result[0].version == "2.0");

    assert(result[1].name == "libfoo");
    assert(result[1].version == "1.0");

    assert(result[2].name == "app");
    assert(result[2].version == "1.0");

    /*
     * Version constraint:
     *
     *     zlib >= 1.5
     *
     * The newest satisfying version is 2.0.
     */
    Package constrained_app{
        "constrained-app",
        "1.0",
        "Constrained application",
        "",
        "",
        "autotools",
        {
            versioned_dependency(
                "zlib",
                VersionConstraint::comparison(
                    ComparisonOperator::GreaterEqual,
                    "1.5"
                )
            )
        }
    };

    const auto constrained_result =
        athena::dependency::resolve(
            constrained_app,
            available
        );

    assert(constrained_result.size() == 2);

    assert(constrained_result[0].name == "zlib");
    assert(constrained_result[0].version == "2.0");

    assert(constrained_result[1].name == "constrained-app");
    assert(constrained_result[1].version == "1.0");

    /*
     * Version range:
     *
     *     zlib >= 1.5 AND zlib < 2.0
     *
     * The newest satisfying version is 1.8.
     */
    Package ranged_app{
        "ranged-app",
        "1.0",
        "Ranged application",
        "",
        "",
        "autotools",
        {
            versioned_dependency(
                "zlib",
                VersionConstraint::all({
                    VersionConstraint::comparison(
                        ComparisonOperator::GreaterEqual,
                        "1.5"
                    ),
                    VersionConstraint::comparison(
                        ComparisonOperator::Less,
                        "2.0"
                    )
                })
            )
        }
    };

    const auto ranged_result =
        athena::dependency::resolve(
            ranged_app,
            available
        );

    assert(ranged_result.size() == 2);

    assert(ranged_result[0].name == "zlib");
    assert(ranged_result[0].version == "1.8");

    assert(ranged_result[1].name == "ranged-app");
    assert(ranged_result[1].version == "1.0");

    /*
     * Impossible version constraint:
     *
     *     zlib >= 3.0
     *
     * No available version satisfies it.
     */
    Package impossible_app{
        "impossible-app",
        "1.0",
        "Impossible application",
        "",
        "",
        "autotools",
        {
            versioned_dependency(
                "zlib",
                VersionConstraint::comparison(
                    ComparisonOperator::GreaterEqual,
                    "3.0"
                )
            )
        }
    };

    bool incompatible_dependency_detected = false;

    try {

        athena::dependency::resolve(
            impossible_app,
            available
        );
    }
    catch (const std::runtime_error&) {

        incompatible_dependency_detected = true;
    }

    assert(incompatible_dependency_detected);

    /*
     * Test missing dependency detection.
     */
    Package broken{
        "broken",
        "1.0",
        "Broken package",
        "",
        "",
        "autotools",
        {
            package_dependency("missing")
        }
    };

    bool missing_dependency_detected = false;

    try {

        athena::dependency::resolve(
            broken,
            available
        );
    }
    catch (const std::runtime_error&) {

        missing_dependency_detected = true;
    }

    assert(missing_dependency_detected);

    /*
     * Test dependency cycle detection.
     */
    Package cycle_a{
        "cycle-a",
        "1.0",
        "Cycle A",
        "",
        "",
        "autotools",
        {
            package_dependency("cycle-b")
        }
    };

    Package cycle_b{
        "cycle-b",
        "1.0",
        "Cycle B",
        "",
        "",
        "autotools",
        {
            package_dependency("cycle-a")
        }
    };

    const std::vector<Package> cyclic_packages{
        cycle_a,
        cycle_b
    };

    bool cycle_detected = false;

    try {

        athena::dependency::resolve(
            cycle_a,
            cyclic_packages
        );
    }
    catch (const std::runtime_error&) {

        cycle_detected = true;
    }

    assert(cycle_detected);

    std::cout
        << "Dependency resolver tests passed.\n";

    return 0;
}
