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

Dependency and_dependency(
    std::vector<ExpressionPtr> expressions
)
{
    return Dependency{
        std::make_shared<const AndExpression>(
            std::move(expressions)
        ),
        DependencyKind::Runtime,
        DependencyContext::Target
    };
}

Dependency or_dependency(
    std::vector<ExpressionPtr> expressions
)
{
    return Dependency{
        std::make_shared<const OrExpression>(
            std::move(expressions)
        ),
        DependencyKind::Runtime,
        DependencyContext::Target
    };
}

Dependency not_dependency(
    ExpressionPtr expression
)
{
    return Dependency{
        std::make_shared<const NotExpression>(
            std::move(expression)
        ),
        DependencyKind::Runtime,
        DependencyContext::Target
    };
}

ExpressionPtr package_expression(
    const std::string& name
)
{
    return std::make_shared<const PackageRef>(name);
}

}

int main()
{
    Package zlib_old{
        "zlib","1.0","Zlib","","","autotools",{}
    };

    Package zlib_mid{
        "zlib","1.8","Zlib","","","autotools",{}
    };

    Package zlib_new{
        "zlib","2.0","Zlib","","","autotools",{}
    };

    Package openssl{
        "openssl","3.0","OpenSSL","","","autotools",{}
    };

    Package curl{
        "curl","8.0","Curl","","","autotools",{}
    };

    Package libfoo{
        "libfoo","1.0","Libfoo","","","autotools",
        { package_dependency("zlib") }
    };

    Package app{
        "app","1.0","Test application","","","autotools",
        { package_dependency("libfoo") }
    };

    const std::vector<Package> available{
        zlib_old,
        zlib_mid,
        zlib_new,
        openssl,
        curl,
        libfoo
    };

    /*
     * Basic dependency resolution.
     */
    const auto result =
        athena::dependency::resolve(app, available);

    assert(result.size() == 3);
    assert(result[0].name == "zlib");
    assert(result[0].version == "2.0");
    assert(result[1].name == "libfoo");
    assert(result[1].version == "1.0");
    assert(result[2].name == "app");
    assert(result[2].version == "1.0");

    /*
     * Version constraint.
     */
    Package constrained_app{
        "constrained-app","1.0","Constrained application","","","autotools",
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
     * Version range.
     */
    Package ranged_app{
        "ranged-app","1.0","Ranged application","","","autotools",
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
     * Impossible version constraint.
     */
    Package impossible_app{
        "impossible-app","1.0","Impossible application","","","autotools",
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
     * Missing dependency.
     */
    Package broken{
        "broken","1.0","Broken package","","","autotools",
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
     * Dependency cycle.
     */
    Package cycle_a{
        "cycle-a","1.0","Cycle A","","","autotools",
        {
            package_dependency("cycle-b")
        }
    };

    Package cycle_b{
        "cycle-b","1.0","Cycle B","","","autotools",
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

    /*
     * Logical AND.
     *
     * app-and depends on both zlib and openssl.
     */
    Package app_and{
        "app-and","1.0","AND application","","","autotools",
        {
            and_dependency({
                package_expression("zlib"),
                package_expression("openssl")
            })
        }
    };

    const auto and_result =
        athena::dependency::resolve(
            app_and,
            available
        );

    assert(and_result.size() == 3);
    assert(and_result[0].name == "zlib");
    assert(and_result[0].version == "2.0");
    assert(and_result[1].name == "openssl");
    assert(and_result[1].version == "3.0");
    assert(and_result[2].name == "app-and");

    /*
     * Logical OR.
     *
     * app-or depends on either zlib or openssl.
     * The first alternative is resolvable and therefore succeeds.
     */
    Package app_or{
        "app-or","1.0","OR application","","","autotools",
        {
            or_dependency({
                package_expression("zlib"),
                package_expression("openssl")
            })
        }
    };

    const auto or_result =
        athena::dependency::resolve(
            app_or,
            available
        );

    assert(or_result.size() == 2);
    assert(or_result[0].name == "zlib");
    assert(or_result[0].version == "2.0");
    assert(or_result[1].name == "app-or");

    /*
     * Logical OR with a failed first alternative.
     *
     * broken-lib exists, but its own dependency is missing.
     * The resolver must therefore try openssl.
     */
    Package broken_lib{
        "broken-lib","1.0","Broken library","","","autotools",
        {
            package_dependency("missing")
        }
    };

    Package fallback_app{
        "fallback-app","1.0","Fallback application","","","autotools",
        {
            or_dependency({
                package_expression("broken-lib"),
                package_expression("openssl")
            })
        }
    };

    const std::vector<Package> fallback_available{
        broken_lib,
        openssl
    };

    const auto fallback_result =
        athena::dependency::resolve(
            fallback_app,
            fallback_available
        );

    assert(fallback_result.size() == 2);
    assert(fallback_result[0].name == "openssl");
    assert(fallback_result[0].version == "3.0");
    assert(fallback_result[1].name == "fallback-app");

    /*
     * Transactional OR rollback.
     *
     * broken-top successfully resolves good-leaf first, but then fails
     * on the missing dependency. The successful part of the failed
     * alternative must be completely rolled back before openssl is tried.
     */
    Package good_leaf{
        "good-leaf","1.0","Good leaf","","","autotools",
        {}
    };

    Package broken_top{
        "broken-top","1.0","Broken top-level package","","","autotools",
        {
            package_dependency("good-leaf"),
            package_dependency("missing")
        }
    };

    Package rollback_app{
        "rollback-app","1.0","Rollback application","","","autotools",
        {
            or_dependency({
                package_expression("broken-top"),
                package_expression("openssl")
            })
        }
    };

    const std::vector<Package> rollback_available{
        broken_top,
        good_leaf,
        openssl
    };

    const auto rollback_result =
        athena::dependency::resolve(
            rollback_app,
            rollback_available
        );

    assert(rollback_result.size() == 2);
    assert(rollback_result[0].name == "openssl");
    assert(rollback_result[0].version == "3.0");
    assert(rollback_result[1].name == "rollback-app");

    /*
     * Logical NOT is parsed by the dependency system,
     * but is intentionally not implemented by the resolver yet.
     */
    Package not_app{
        "not-app","1.0","NOT application","","","autotools",
        {
            not_dependency(
                package_expression("zlib")
            )
        }
    };

    bool not_unsupported_detected = false;

    try {
        athena::dependency::resolve(
            not_app,
            available
        );
    }
    catch (const std::runtime_error&) {
        not_unsupported_detected = true;
    }

    assert(not_unsupported_detected);

    std::cout << "Dependency resolver tests passed.\n";
    return 0;
}
