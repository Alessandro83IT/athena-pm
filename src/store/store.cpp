#include "store.hpp"

#include "../paths/paths.hpp"

#include <filesystem>
#include <stdexcept>

namespace athena::store {

std::filesystem::path install(
    const std::filesystem::path& staging_directory,
    const std::string& package_name,
    const std::string& package_version
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

    return destination;
}

}
