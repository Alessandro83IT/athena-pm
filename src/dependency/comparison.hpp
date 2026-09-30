#pragma once

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

} // namespace athena::dependency
