#include "installer.hpp"

#include "../package/package.hpp"
#include "../package/package_loader.hpp"
#include "../download/downloader.hpp"
#include "../verify/verifier.hpp"
#include "../paths/paths.hpp"
#include "../archive/archive.hpp"
#include "../build/builder.hpp"
#include "../store/store.hpp"

#include <iostream>
#include <stdexcept>

namespace athena::install {

void install_package(
    const std::filesystem::path& package_file
)
{
    const athena::package::Package package =
        athena::package::load_from_file(package_file);

    std::cout
        << "Installazione di: " << package.name
        << " " << package.version << '\n';

    const std::filesystem::path destination =
        athena::paths::downloads() /
        ("athena-" + package.name + "-" + package.version + ".tar.gz");

    std::cout
        << "Download: " << package.source << '\n';

    const auto downloaded =
        athena::download::download_file(
            package.source,
            destination
        );

    std::cout
        << "Download completato: "
        << downloaded << '\n';

    std::cout << "Verifica SHA-256...\n";

    const bool valid =
        athena::verify::verify_sha256(
            downloaded,
            package.sha256
        );

    if (!valid) {
        throw std::runtime_error(
            "SHA-256 non corrisponde per: " +
            downloaded.string()
        );
    }

    std::cout << "SHA-256 verificato correttamente.\n";

    const std::filesystem::path source_directory =
        athena::paths::sources();

    std::cout
        << "Estrazione in: "
        << source_directory << '\n';

    const auto extracted =
        athena::archive::extract(
            downloaded,
            source_directory
        );

    std::cout
        << "Estrazione completata: "
        << extracted << '\n';

    std::cout
        << "Compilazione...\n";

    athena::build::build(extracted);

    std::cout
        << "Compilazione completata.\n";

    const std::filesystem::path staging =
        athena::paths::build() /
        (package.name + "-" + package.version + "-install");

    std::cout
        << "Installazione nello staging: "
        << staging << '\n';

    athena::build::install(
        extracted,
        staging
    );

    std::cout
        << "Installazione nello staging completata.\n";

    const auto installed =
        athena::store::install(
            staging,
            package.name,
            package.version,
            package.source,
            package.sha256,
            "autotools"
    );    

    std::cout
        << "Installazione nello store completata: "
        << installed << '\n';

}

}
