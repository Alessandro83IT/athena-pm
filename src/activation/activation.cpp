#include "activation.hpp"

#include "../store/manifest.hpp"

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

    const auto manifest =
        athena::store::read_manifest(
            store_directory
        );

    for (const auto& entry : manifest) {

        const std::filesystem::path source =
            store_directory / entry.path;

        const std::filesystem::path target =
            std::filesystem::path("/usr") /
            entry.path;

        std::cout
            << "  File: "
            << target
            << '\n';
    }
}

}
