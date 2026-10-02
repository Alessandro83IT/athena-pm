#pragma once

#include "expression.hpp"

#include <string>

namespace athena::dependency {

/*
 * Parse a dependency expression from its textual representation.
 *
 * Supported forms:
 *
 *   package
 *   package = version
 *   package != version
 *   package < version
 *   package <= version
 *   package > version
 *   package >= version
 */
Dependency parse_dependency(const std::string& text);

} // namespace athena::dependency
