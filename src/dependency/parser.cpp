#include "parser.hpp"

#include "constraint.hpp"

#include <memory>
#include <stdexcept>
#include <string>

namespace athena::dependency {

namespace {

std::string trim(const std::string& text)
{
    const auto first =
        text.find_first_not_of(" \t");

    if (first == std::string::npos) {
        return "";
    }

    const auto last =
        text.find_last_not_of(" \t");

    return text.substr(
        first,
        last - first + 1
    );
}

struct ParsedDependency {
    std::string package;
    ComparisonOperator op;
    std::string version;
    bool has_constraint;
};

ParsedDependency parse_text(const std::string& text)
{
    const std::string input = trim(text);

    if (input.empty()) {
        throw std::runtime_error(
            "Espressione di dipendenza vuota"
        );
    }

    const auto first_space =
        input.find_first_of(" \t");

    if (first_space == std::string::npos) {
        return {
            input,
            ComparisonOperator::Equal,
            "",
            false
        };
    }

    const std::string package =
        input.substr(0, first_space);

    const std::string constraint =
        trim(input.substr(first_space));

    if (constraint.empty()) {
        throw std::runtime_error(
            "Vincolo di versione mancante per il pacchetto '" +
            package + "'"
        );
    }

    ComparisonOperator op;

    std::size_t operator_length = 0;

    if (constraint.starts_with(">=")) {
        op = ComparisonOperator::GreaterEqual;
        operator_length = 2;
    }
    else if (constraint.starts_with("<=")) {
        op = ComparisonOperator::LessEqual;
        operator_length = 2;
    }
    else if (constraint.starts_with("!=")) {
        op = ComparisonOperator::NotEqual;
        operator_length = 2;
    }
    else if (constraint.starts_with("=")) {
        op = ComparisonOperator::Equal;
        operator_length = 1;
    }
    else if (constraint.starts_with(">")) {
        op = ComparisonOperator::Greater;
        operator_length = 1;
    }
    else if (constraint.starts_with("<")) {
        op = ComparisonOperator::Less;
        operator_length = 1;
    }
    else {
        throw std::runtime_error(
            "Operatore di versione non valido nella dipendenza '" +
            text + "'"
        );
    }

    const std::string version =
        trim(constraint.substr(operator_length));

    if (version.empty()) {
        throw std::runtime_error(
            "Versione mancante nella dipendenza '" +
            text + "'"
        );
    }

    return {
        package,
        op,
        version,
        true
    };
}

}

Dependency parse_dependency(const std::string& text)
{
    const auto parsed = parse_text(text);

    auto package =
        std::make_shared<const PackageRef>(
            parsed.package
        );

    ExpressionPtr expression = package;

    if (parsed.has_constraint) {
        auto constraint =
            std::make_shared<const VersionConstraint>(
                VersionConstraint::comparison(
                    parsed.op,
                    parsed.version
                )
            );

        expression =
            std::make_shared<const VersionComparison>(
                package,
                constraint
            );
    }

    return Dependency{
        expression,
        DependencyKind::Runtime,
        DependencyContext::Target
    };
}

} // namespace athena::dependency
