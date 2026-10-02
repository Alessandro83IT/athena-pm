#include "parser.hpp"

#include "constraint.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace athena::dependency {

namespace {

class Parser {
public:

    explicit Parser(const std::string& input)
        : text(input)
    {
    }

    ExpressionPtr parse()
    {
        skip_whitespace();

        if (position >= text.size()) {
            throw std::runtime_error(
                "Espressione di dipendenza vuota"
            );
        }

        auto expression = parse_or();

        skip_whitespace();

        if (position != text.size()) {
            throw std::runtime_error(
                "Token inatteso nella dipendenza '" +
                text +
                "'"
            );
        }

        return expression;
    }

private:

    ExpressionPtr parse_or()
    {
        std::vector<ExpressionPtr> expressions;

        expressions.push_back(parse_and());

        while (consume_keyword("OR")) {
            expressions.push_back(parse_and());
        }

        if (expressions.size() == 1) {
            return expressions.front();
        }

        return std::make_shared<const OrExpression>(
            std::move(expressions)
        );
    }

    ExpressionPtr parse_and()
    {
        std::vector<ExpressionPtr> expressions;

        expressions.push_back(parse_unary());

        while (consume_keyword("AND")) {
            expressions.push_back(parse_unary());
        }

        if (expressions.size() == 1) {
            return expressions.front();
        }

        return std::make_shared<const AndExpression>(
            std::move(expressions)
        );
    }

    ExpressionPtr parse_unary()
    {
        if (consume_keyword("NOT")) {
            return std::make_shared<const NotExpression>(
                parse_unary()
            );
        }

        return parse_primary();
    }

    ExpressionPtr parse_primary()
    {
        skip_whitespace();

        if (consume_character('(')) {

            auto expression = parse_or();

            skip_whitespace();

            if (!consume_character(')')) {
                throw std::runtime_error(
                    "Parentesi ')' mancante nella dipendenza '" +
                    text +
                    "'"
                );
            }

            return expression;
        }

        return parse_dependency_atom();
    }

    ExpressionPtr parse_dependency_atom()
    {
        const std::string package = parse_identifier();

        if (package.empty()) {
            throw std::runtime_error(
                "Nome del pacchetto mancante nella dipendenza '" +
                text +
                "'"
            );
        }

        skip_whitespace();

        if (position >= text.size() ||
            text[position] == ')' ||
            starts_with_keyword("AND") ||
            starts_with_keyword("OR"))
        {
            return std::make_shared<const PackageRef>(
                package
            );
        }

        const auto op = parse_operator();

        skip_whitespace();

        const std::string version = parse_version();

        if (version.empty()) {
            throw std::runtime_error(
                "Versione mancante nella dipendenza '" +
                text +
                "'"
            );
        }

        auto package_reference =
            std::make_shared<const PackageRef>(
                package
            );

        auto constraint =
            std::make_shared<const VersionConstraint>(
                VersionConstraint::comparison(
                    op,
                    version
                )
            );

        return std::make_shared<const VersionComparison>(
            package_reference,
            constraint
        );
    }

    std::string parse_identifier()
    {
        skip_whitespace();

        const std::size_t start = position;

        while (position < text.size()) {

            const char character = text[position];

            if (character == ' ' ||
                character == '\t' ||
                character == '(' ||
                character == ')' ||
                character == '<' ||
                character == '>' ||
                character == '=' ||
                character == '!')
            {
                break;
            }

            ++position;
        }

        return text.substr(
            start,
            position - start
        );
    }

    std::string parse_version()
    {
        const std::size_t start = position;

        while (position < text.size()) {

            if (text[position] == ')') {
                break;
            }

            if (starts_with_keyword("AND") ||
                starts_with_keyword("OR"))
            {
                break;
            }

            ++position;
        }

        std::string version =
            text.substr(
                start,
                position - start
            );

        return trim(version);
    }

    ComparisonOperator parse_operator()
    {
        skip_whitespace();

        if (consume_string(">=")) {
            return ComparisonOperator::GreaterEqual;
        }

        if (consume_string("<=")) {
            return ComparisonOperator::LessEqual;
        }

        if (consume_string("!=")) {
            return ComparisonOperator::NotEqual;
        }

        if (consume_string("=")) {
            return ComparisonOperator::Equal;
        }

        if (consume_string(">")) {
            return ComparisonOperator::Greater;
        }

        if (consume_string("<")) {
            return ComparisonOperator::Less;
        }

        throw std::runtime_error(
            "Operatore di versione non valido nella dipendenza '" +
            text +
            "'"
        );
    }

    bool consume_keyword(const std::string& keyword)
    {
        skip_whitespace();

        if (!starts_with_keyword(keyword)) {
            return false;
        }

        position += keyword.size();

        return true;
    }

    bool starts_with_keyword(
        const std::string& keyword
    ) const
    {
        if (text.compare(
                position,
                keyword.size(),
                keyword
            ) != 0)
        {
            return false;
        }

        const std::size_t end =
            position + keyword.size();

        if (end < text.size()) {

            const char next = text[end];

            if (next != ' ' &&
                next != '\t' &&
                next != '(' &&
                next != ')')
            {
                return false;
            }
        }

        return true;
    }

    bool consume_character(char character)
    {
        skip_whitespace();

        if (position >= text.size() ||
            text[position] != character)
        {
            return false;
        }

        ++position;

        return true;
    }

    bool consume_string(const std::string& value)
    {
        if (text.compare(
                position,
                value.size(),
                value
            ) != 0)
        {
            return false;
        }

        position += value.size();

        return true;
    }

    void skip_whitespace()
    {
        while (position < text.size() &&
               (text[position] == ' ' ||
                text[position] == '\t'))
        {
            ++position;
        }
    }

    static std::string trim(
        const std::string& value
    )
    {
        const auto first =
            value.find_first_not_of(" \t");

        if (first == std::string::npos) {
            return "";
        }

        const auto last =
            value.find_last_not_of(" \t");

        return value.substr(
            first,
            last - first + 1
        );
    }

    const std::string& text;
    std::size_t position = 0;
};

}

Dependency parse_dependency(const std::string& text)
{
    Parser parser(text);

    return Dependency{
        parser.parse(),
        DependencyKind::Runtime,
        DependencyContext::Target
    };
}

} // namespace athena::dependency
