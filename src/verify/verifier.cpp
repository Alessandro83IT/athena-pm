#include "verifier.hpp"

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

struct PipeCloser {
    void operator()(FILE* pipe) const
    {
        if (pipe != nullptr) {
            pclose(pipe);
        }
    }
};

}

namespace athena::verify {

bool verify_sha256(
    const std::filesystem::path& file,
    const std::string& expected_hash
)
{
    const std::string command =
        "sha256sum \"" + file.string() + "\"";

    std::unique_ptr<FILE, PipeCloser> pipe(
        popen(command.c_str(), "r")
    );
 
    if (!pipe) {
        throw std::runtime_error(
            "Impossibile calcolare SHA-256 per: " + file.string()
        );
    }

    char buffer[256];

    if (!fgets(buffer, sizeof(buffer), pipe.get())) {
        throw std::runtime_error(
            "Impossibile leggere SHA-256 per: " + file.string()
        );
    }

    const std::string output(buffer);

    const std::string calculated_hash =
        output.substr(0, output.find(' '));

    return calculated_hash == expected_hash;
}

}
