#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace athena::dependency {

/*
 * Operators used by version constraints.
 */
enum class ComparisonOperator {
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual
};

/*
 * Semantic role of a dependency.
 *
 * These roles are intentionally broader than the current
 * Package::dependencies representation. They allow Athena to
 * represent dependency semantics from different package
 * ecosystems.
 */
enum class DependencyKind {
    Runtime,
    Build,
    Test,
    Optional,
    Development,
    Propagated
};

/*
 * Context in which a dependency is required.
 *
 * Host   : dependency available on the host system.
 * Build  : dependency used while producing the package.
 * Target : dependency required by the resulting package.
 */
enum class DependencyContext {
    Host,
    Build,
    Target
};

/*
 * Base class for Athena dependency expressions.
 *
 * An expression describes what can satisfy a dependency.
 *
 * PackageRef and CapabilityRef are references.
 * VersionComparison and the logical expressions operate on
 * those references.
 */
struct Expression {

    enum class Kind {
        True,
        False,

        Package,
        Capability,
        Feature,
        Platform,
        Architecture,
        ABI,

        VersionComparison,

        And,
        Or,
        Not,

        Conditional
    };

    virtual ~Expression() = default;

    Kind kind;

protected:

    explicit Expression(Kind expression_kind)
        : kind(expression_kind)
    {
    }
};

using ExpressionPtr =
    std::shared_ptr<const Expression>;

/*
 * Boolean constants.
 */
struct TrueExpression final : Expression {

    TrueExpression()
        : Expression(Kind::True)
    {
    }
};

struct FalseExpression final : Expression {

    FalseExpression()
        : Expression(Kind::False)
    {
    }
};

/*
 * References to packages and capabilities.
 */
struct PackageRef final : Expression {

    explicit PackageRef(std::string package_name)
        : Expression(Kind::Package),
          name(std::move(package_name))
    {
    }

    std::string name;
};

struct CapabilityRef final : Expression {

    explicit CapabilityRef(std::string capability_name)
        : Expression(Kind::Capability),
          name(std::move(capability_name))
    {
    }

    std::string name;
};

/*
 * Feature, platform, architecture and ABI references.
 *
 * These are references in the semantic model. Their precise
 * interpretation will be defined by the resolver and by the
 * package/platform context.
 */
struct FeatureRef final : Expression {

    explicit FeatureRef(std::string feature_name)
        : Expression(Kind::Feature),
          name(std::move(feature_name))
    {
    }

    std::string name;
};

struct PlatformRef final : Expression {

    explicit PlatformRef(std::string platform_name)
        : Expression(Kind::Platform),
          name(std::move(platform_name))
    {
    }

    std::string name;
};

struct ArchitectureRef final : Expression {

    explicit ArchitectureRef(std::string architecture_name)
        : Expression(Kind::Architecture),
          name(std::move(architecture_name))
    {
    }

    std::string name;
};

struct ABIRef final : Expression {

    explicit ABIRef(std::string abi_name)
        : Expression(Kind::ABI),
          name(std::move(abi_name))
    {
    }

    std::string name;
};

/*
 * Version constraint applied to another expression.
 *
 * Example:
 *
 *     Package("zlib") >= "1.3"
 */
struct VersionComparison final : Expression {

    VersionComparison(
        ExpressionPtr target_expression,
        ComparisonOperator comparison_operator,
        std::string comparison_version
    )
        : Expression(Kind::VersionComparison),
          target(std::move(target_expression)),
          op(comparison_operator),
          version(std::move(comparison_version))
    {
    }

    ExpressionPtr target;

    ComparisonOperator op;

    std::string version;
};

/*
 * Logical conjunction.
 *
 * All expressions must be satisfied.
 */
struct AndExpression final : Expression {

    explicit AndExpression(
        std::vector<ExpressionPtr> expression_list
    )
        : Expression(Kind::And),
          expressions(std::move(expression_list))
    {
    }

    std::vector<ExpressionPtr> expressions;
};

/*
 * Logical disjunction.
 *
 * At least one expression must be satisfied.
 */
struct OrExpression final : Expression {

    explicit OrExpression(
        std::vector<ExpressionPtr> expression_list
    )
        : Expression(Kind::Or),
          expressions(std::move(expression_list))
    {
    }

    std::vector<ExpressionPtr> expressions;
};

/*
 * Logical negation.
 */
struct NotExpression final : Expression {

    explicit NotExpression(ExpressionPtr expression)
        : Expression(Kind::Not),
          expression(std::move(expression))
    {
    }

    ExpressionPtr expression;
};

/*
 * Conditional dependency expression.
 *
 * The body becomes active when the condition is satisfied.
 *
 * This will eventually allow Athena to model conditional
 * dependencies such as Gentoo USE-flag based dependencies.
 */
struct ConditionalExpression final : Expression {

    ConditionalExpression(
        ExpressionPtr condition_expression,
        ExpressionPtr body_expression
    )
        : Expression(Kind::Conditional),
          condition(std::move(condition_expression)),
          body(std::move(body_expression))
    {
    }

    ExpressionPtr condition;

    ExpressionPtr body;
};

/*
 * A dependency combines an expression with its semantic role
 * and dependency context.
 */
struct Dependency {

    ExpressionPtr expression;

    DependencyKind kind;

    DependencyContext context;
};

} // namespace athena::dependency
