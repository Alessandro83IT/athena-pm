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

void deactivate(
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
        << "Disattivazione del pacchetto: "
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
            target_root / entry.path;

        const auto status =
            std::filesystem::symlink_status(target);

        if (status.type() ==
            std::filesystem::file_type::not_found) {
            continue;
        }

        if (status.type() !=
            std::filesystem::file_type::symlink) {
            throw std::runtime_error(
                "Impossibile disattivare: il target non è un symlink: " +
                target.string()
            );
        }

        const auto linked_target =
            std::filesystem::read_symlink(target);

        const auto expected_target =
            std::filesystem::absolute(source);

        const auto actual_target =
            std::filesystem::absolute(
                target.parent_path() / linked_target
            );

        if (actual_target != expected_target) {
            throw std::runtime_error(
                "Impossibile disattivare: il symlink non punta allo store previsto: " +
                target.string()
            );
        }

        std::cout
            << "  Rimozione: "
            << target
            << '\n';

        std::filesystem::remove(target);
    }
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

    /*
     * Keep track of every symlink successfully created by this
     * activation transaction. If a later operation fails, these
     * targets can be removed without touching pre-existing objects.
     */
    std::vector<std::filesystem::path> activated_targets;

    try {

        for (const auto& file : files) {

            std::cout
                << "  File: "
                << file.second
                << '\n';

            activate_file(
                file.first,
                file.second
            );

            /*
             * Record the target only after successful creation.
             * Therefore rollback can remove only objects created
             * by this activation transaction.
             */
            activated_targets.push_back(
                file.second
            );
        }
    }
    catch (...) {

        /*
         * Undo the transaction in reverse order. We intentionally
         * ignore cleanup errors here so that the original exception
         * remains the one reported to the caller.
         */
        for (
            auto it = activated_targets.rbegin();
            it != activated_targets.rend();
            ++it
        ) {
            std::error_code error;

            std::filesystem::remove(
                *it,
                error
            );
        }

        /*
         * Preserve the original activation failure.
         */
        throw;
    }
  
}

}


