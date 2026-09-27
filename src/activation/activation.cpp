#include "activation.hpp"

#include <iostream>
#include <stdexcept>

namespace athena::activation {

void activate(
    const std::filesystem::path& store_directory
)
{
    if (!std::filesystem::exists(store_directory)) {
        throw std::runtime_error(
            "Directory dello store non trovata: " +
            store_directory.string()
        );
    }

    std::cout
        << "Attivazione del pacchetto: "
        << store_directory.string()
        << '\n';

    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(store_directory)) {

        if (!entry.is_regular_file()) {
            continue;
        }

        const auto relative =
            std::filesystem::relative(
                entry.path(),
                store_directory
            );

        if (relative.begin() != relative.end() &&
            *relative.begin() == ".athena") {
            continue;
        }

        std::cout
            << "  File: /usr/"
            << relative.string()
            << '\n';
    }
}

}
