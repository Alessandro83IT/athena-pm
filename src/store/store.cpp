#include "store.hpp"

#include "../paths/paths.hpp"
#include "metadata.hpp"

#include <filesystem>
#include <stdexcept>

namespace athena::store {

std::filesystem::path install(
    const std::filesystem::path& staging_directory,
    const std::string& package_name,
    const std::string& package_version,
    const std::string& source,
    const std::string& sha256,
    const std::string& build_system
)
{
    const std::filesystem::path destination =
        athena::paths::store() /
        (package_name + "-" + package_version);

    if (std::filesystem::exists(destination)) {
        throw std::runtime_error(
            "Il pacchetto è già presente nello store: " +
            destination.string()
        );
    }

    const std::filesystem::path installed_root =
        staging_directory / "usr";

    if (!std::filesystem::is_directory(installed_root)) {
        throw std::runtime_error(
            "Directory di installazione non trovata: " +
            installed_root.string()
        );
    }

    std::filesystem::create_directories(destination);

    for (const auto& entry :
         std::filesystem::directory_iterator(installed_root)) {

        std::filesystem::copy(
            entry.path(),
            destination / entry.path().filename(),
            std::filesystem::copy_options::recursive
        );
    }

    const Metadata metadata{
        package_name,
        package_version,
        source,
        sha256,
        build_system
    };

    write_metadata(
        destination,
        metadata
    );

    return destination;
}

std::filesystem::path find(
    const std::string& package_name
)
{
    const std::filesystem::path store_directory =
        athena::paths::store();

    if (!std::filesystem::exists(store_directory)) {
        throw std::runtime_error(
            "Store non trovato: " +
            store_directory.string()
        );
    }

    for (const auto& entry :
         std::filesystem::directory_iterator(store_directory)) {

        if (!entry.is_directory()) {
            continue;
        }

        const std::string expected_prefix =
            package_name + "-";

        const std::string directory_name =
            entry.path().filename().string();

        if (directory_name.rfind(expected_prefix, 0) != 0) {
            continue;
        }

        const std::filesystem::path metadata_file =
            entry.path() /
            ".athena" /
            "metadata.toml";

        if (std::filesystem::exists(metadata_file)) {
            return entry.path();
        }
    }

    throw std::runtime_error(
        "Pacchetto non installato: " +
        package_name
    );
}

}
