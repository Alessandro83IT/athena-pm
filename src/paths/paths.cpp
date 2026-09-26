#include "paths.hpp"

namespace athena::paths {

std::filesystem::path root()
{
    return "/var/lib/athena";
}

std::filesystem::path downloads()
{
    return root() / "downloads";
}

std::filesystem::path sources()
{
    return root() / "sources";
}

std::filesystem::path build()
{
    return root() / "build";
}

std::filesystem::path store()
{
    return root() / "store";
}

void initialize()
{
    std::filesystem::create_directories(downloads());
    std::filesystem::create_directories(sources());
    std::filesystem::create_directories(build());
    std::filesystem::create_directories(store());
}

}
