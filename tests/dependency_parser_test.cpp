#include "../src/dependency/parser.hpp"
#include "../src/dependency/constraint.hpp"

#include <cassert>
#include <iostream>

using namespace athena::dependency;

namespace {

const PackageRef* get_package_ref(
    const Dependency& dependency
)
{
    return dynamic_cast<const PackageRef*>(
        dependency.expression.get()
    );
}

const VersionComparison* get_version_comparison(
    const Dependency& dependency
)
{
    return dynamic_cast<const VersionComparison*>(
        dependency.expression.get()
    );
}

}

int main()
{
    {
        const auto dependency =
            parse_dependency("zlib");

        assert(dependency.kind == DependencyKind::Runtime);
        assert(dependency.context == DependencyContext::Target);
        assert(dependency.expression != nullptr);
        assert(
            dependency.expression->kind ==
            Expression::Kind::Package
        );

        const auto* package =
            get_package_ref(dependency);

        assert(package != nullptr);
        assert(package->name == "zlib");
    }

    {
        const auto dependency =
            parse_dependency("zlib >= 1.2");

        assert(
            dependency.expression->kind ==
            Expression::Kind::VersionComparison
        );

        const auto* comparison =
            get_version_comparison(dependency);

        assert(comparison != nullptr);
        assert(comparison->target != nullptr);
        assert(comparison->constraint != nullptr);

        const auto* package =
            dynamic_cast<const PackageRef*>(
                comparison->target.get()
            );

        assert(package != nullptr);
        assert(package->name == "zlib");

        assert(
            comparison->constraint->kind ==
            VersionConstraint::Kind::Comparison
        );

        assert(
            comparison->constraint->comparison_value.op ==
            ComparisonOperator::GreaterEqual
        );

        assert(
            comparison->constraint->comparison_value.version ==
            "1.2"
        );
    }

    {
        const auto dependency =
            parse_dependency("openssl < 4.0");

        const auto* comparison =
            get_version_comparison(dependency);

        assert(comparison != nullptr);

        assert(
            comparison->constraint->comparison_value.op ==
            ComparisonOperator::Less
        );

        assert(
            comparison->constraint->comparison_value.version ==
            "4.0"
        );
    }

    {
        const auto dependency =
            parse_dependency("cmake != 3.30");

        const auto* comparison =
            get_version_comparison(dependency);

        assert(comparison != nullptr);

        assert(
            comparison->constraint->comparison_value.op ==
            ComparisonOperator::NotEqual
        );

        assert(
            comparison->constraint->comparison_value.version ==
            "3.30"
        );
    }

    {
        bool failed = false;

        try {
            parse_dependency("zlib >");
        }
        catch (const std::runtime_error&) {
            failed = true;
        }

        assert(failed);
    }

    {
        bool failed = false;

        try {
            parse_dependency("");
        }
        catch (const std::runtime_error&) {
            failed = true;
        }

        assert(failed);
    }

    std::cout
        << "Dependency parser tests passed.\n";

    return 0;
}
