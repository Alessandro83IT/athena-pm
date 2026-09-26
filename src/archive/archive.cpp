#include "archive.hpp"

#include <cstdlib>
#include <stdexcept>

namespace athena::archive {

std::filesystem::path extract(
    const std::filesystem::path& archive,
    const std::filesystem::path& destination
)
{
    std::filesystem::create_directories(destination);

    const std::string command =
        "tar -xzf \"" + archive.string() +
        "\" -C \"" + destination.string() + "\"";

    const int result = std::system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(
            "Estrazione fallita: " + archive.string()
        );
    }

    std::filesystem::path extracted_directory;

    for (const auto& entry :
         std::filesystem::directory_iterator(destination)) {

        if (entry.is_directory()) {
            if (!extracted_directory.empty()) {
                throw std::runtime_error(
                    "L'archivio contiene più directory principali: " +
                    archive.string()
                );
            }

            extracted_directory = entry.path();
        }
    }

    if (extracted_directory.empty()) {
        throw std::runtime_error(
            "Nessuna directory principale trovata dopo l'estrazione: " +
            archive.string()
        );
    }

    return extracted_directory;
}

}
