#include "archive.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace athena::archive {

/*
 * Extract a package source archive.
 *
 * The current implementation uses the system tar utility and
 * supports gzip-compressed tar archives (.tar.gz).
 *
 * After extraction, Athena expects the archive to contain exactly
 * one top-level directory. That directory is returned as the source
 * directory used by the build system.
 */
std::filesystem::path extract(
    const std::filesystem::path& archive,
    const std::filesystem::path& destination
)
{
    /*
     * Ensure that the extraction destination exists before running
     * tar.
     */
    std::filesystem::create_directories(destination);

    /*
     * Extract the archive into the destination directory.
     *
     * -x    extract files
     * -z    decompress gzip data
     * -f    specify the archive file
     * -C    change to the destination directory before extraction
     */
    const std::string command =
        "tar -xzf \"" + archive.string() +
        "\" -C \"" + destination.string() + "\"";

    const int result = std::system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(
            "Estrazione fallita: " + archive.string()
        );
    }

    /*
     * Find the top-level directory created by the extraction.
     *
     * Athena currently expects exactly one package source directory
     * at the root of the extracted archive.
     */
    std::filesystem::path extracted_directory;

    for (const auto& entry :
         std::filesystem::directory_iterator(destination)) {

        if (entry.is_directory()) {

            /*
             * More than one top-level directory would make the source
             * layout ambiguous for the current implementation.
             */
            if (!extracted_directory.empty()) {
                throw std::runtime_error(
                    "L'archivio contiene più directory principali: " +
                    archive.string()
                );
            }

            extracted_directory = entry.path();
        }
    }

    /*
     * An archive without a top-level directory is not compatible
     * with the current source extraction model.
     */
    if (extracted_directory.empty()) {
        throw std::runtime_error(
            "Nessuna directory principale trovata dopo l'estrazione: " +
            archive.string()
        );
    }

    return extracted_directory;
}

}
