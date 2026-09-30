#include "version.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace athena::version {

namespace {

std::vector<unsigned long long> parse(
    const std::string& version
)
{
    if (version.empty()) {
        throw std::runtime_error(
            "Versione vuota"
        );
    }

    std::vector<unsigned long long> parts;

    std::stringstream stream(version);
    std::string part;

    while (std::getline(stream, part, '.')) {

        if (part.empty()) {
            throw std::runtime_error(
                "Versione non valida: " +
                version
            );
        }

        try {
            std::size_t position = 0;

            const auto value =
                std::stoull(part, &position);

            if (position != part.size()) {
                throw std::runtime_error(
                    "Versione non valida: " +
                    version
                );
            }

            parts.push_back(value);
        }
        catch (const std::exception&) {
            throw std::runtime_error(
                "Versione non valida: " +
                version
            );
        }
    }

    return parts;
}

}

bool less(
    const std::string& left,
    const std::string& right
)
{
    const auto left_parts = parse(left);
    const auto right_parts = parse(right);

    const std::size_t count =
        std::max(
            left_parts.size(),
            right_parts.size()
        );

    for (std::size_t i = 0; i < count; ++i) {

        const auto left_value =
            i < left_parts.size()
                ? left_parts[i]
                : 0;

        const auto right_value =
            i < right_parts.size()
                ? right_parts[i]
                : 0;

        if (left_value < right_value) {
            return true;
        }

        if (left_value > right_value) {
            return false;
        }
    }

    return false;
}

bool greater(
    const std::string& left,
    const std::string& right
)
{
    return less(right, left);
}

}
