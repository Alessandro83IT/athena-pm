#include "metadata.hpp"

#include <fstream>
#include <stdexcept>
#include <toml++/toml.hpp>

namespace athena::store {

void write_metadata(
    const std::filesystem::path& store_directory,
    const Metadata& metadata
)
{
    const std::filesystem::path metadata_directory =
        store_directory / ".athena";

    std::filesystem::create_directories(metadata_directory);

    const std::filesystem::path metadata_file =
        metadata_directory / "metadata.toml";

    std::ofstream output(metadata_file);

    if (!output) {
        throw std::runtime_error(
            "Impossibile creare il file dei metadati: " +
            metadata_file.string()
        );
    }

    output
        << "name = \"" << metadata.name << "\"\n"
        << "version = \"" << metadata.version << "\"\n"
        << "source = \"" << metadata.source << "\"\n"
        << "sha256 = \"" << metadata.sha256 << "\"\n"
        << "build_system = \"" << metadata.build_system << "\"\n";
}

Metadata read_metadata(
    const std::filesystem::path& store_directory
)
{
    const std::filesystem::path metadata_file =
        store_directory /
        ".athena" /
        "metadata.toml";

    try {
        const auto table =
            toml::parse_file(metadata_file.string());

        return Metadata{
            table["name"].value_or(""),
            table["version"].value_or(""),
            table["source"].value_or(""),
            table["sha256"].value_or(""),
            table["build_system"].value_or("")
        };
    }
    catch (const toml::parse_error& error) {
        throw std::runtime_error(
            "Impossibile leggere i metadati del pacchetto '" +
            store_directory.string() + "': " +
            error.what()
        );
    }
}

}
