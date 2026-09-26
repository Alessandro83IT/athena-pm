#include "builder.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace athena::build {

void build(
    const std::filesystem::path& source_directory
)
{
    const std::string command =
        "cd \"" + source_directory.string() +
        "\" && ./configure --prefix=/usr && make";

    const int result = std::system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(
            "Compilazione fallita in: " +
            source_directory.string()
        );
    }
}

void install(
    const std::filesystem::path& source_directory,
    const std::filesystem::path& staging_directory
)
{
    std::filesystem::remove_all(staging_directory);
    std::filesystem::create_directories(staging_directory);

    const std::string command =
        "cd \"" + source_directory.string() +
        "\" && make install DESTDIR=\"" +
        staging_directory.string() + "\"";

    const int result = std::system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(
            "Installazione fallita in: " +
            source_directory.string()
        );
    }
}

}
