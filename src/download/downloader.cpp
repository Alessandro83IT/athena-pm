#include "downloader.hpp"

#include <cstdlib>
#include <stdexcept>

namespace athena::download {

std::filesystem::path download_file(
    const std::string& url,
    const std::filesystem::path& destination
)
{
    const std::string command =
        "curl -L --fail --silent --show-error "
        "-o \"" + destination.string() + "\" "
        "\"" + url + "\"";

    const int result = std::system(command.c_str());

    if (result != 0) {
        throw std::runtime_error(
            "Download fallito: " + url
        );
    }

    return destination;
}

}
