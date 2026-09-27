#pragma once

#include <filesystem>
#include <string>

namespace athena::download {

/*
 * Download a file from a remote URL.
 *
 * The downloaded file is written to the destination path.
 *
 * The downloader is intentionally independent from the package
 * definition: it receives only the URL and the destination.
 */
std::filesystem::path download_file(
    const std::string& url,
    const std::filesystem::path& destination
);

}
