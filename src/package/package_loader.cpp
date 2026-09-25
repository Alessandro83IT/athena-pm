#include "package_loader.hpp"

#include <toml++/toml.hpp>
#include <stdexcept>

namespace athena::package {

Package load_from_file(const std::filesystem::path& path)
{
    try {
        const auto table = toml::parse_file(path.string());

        return Package{
            table["name"].value_or(""),
            table["version"].value_or(""),
            table["description"].value_or(""),
            table["source"].value_or(""),
            table["sha256"].value_or("")
        };
    }
    catch (const toml::parse_error& error) {
        throw std::runtime_error(
            "Impossibile caricare il pacchetto '" +
            path.string() + "': " + error.what()
        );
    }
}

}
