#include "activation.hpp"

#include "../store/manifest.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace athena::activation {

void activate_file(
    const std::filesystem::path& source,
    const std::filesystem::path& target
)
{
    const auto parent =
        target.parent_path();

    if (!std::filesystem::exists(parent)) {
        std::filesystem::create_directories(parent);
    }

    std::filesystem::create_symlink(
        source,
        target
    );
}

bool target_is_free(
    const std::filesystem::path& target
)
{
    const auto status =
        std::filesystem::symlink_status(target);

    return status.type() ==
           std::filesystem::file_type::not_found;
}

void activate(
    const std::filesystem::path& store_directory,
    const std::filesystem::path& target_root
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

    std::vector<
        std::pair<
            std::filesystem::path,
            std::filesystem::path
        >
    > files;

    for (const auto& entry : manifest) {

        const std::filesystem::path source =
            store_directory / entry.path;

        const std::filesystem::path target =
            target_root /
            entry.path;

        if (!target_is_free(target)) {
            throw std::runtime_error(
                "Conflitto: il file di destinazione esiste già: " +
                target.string()
            );
        }

        files.emplace_back(
            source,
            target
        );
    }

    for (const auto& file : files) {

        std::cout
            << "  File: "
            << file.second
            << '\n';

        activate_file(
            file.first,
            file.second
        );
    }
  
}

}


