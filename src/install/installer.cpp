#include "installer.hpp"

#include "../package/package.hpp"
#include "../package/package_loader.hpp"
#include "../download/downloader.hpp"
#include "../verify/verifier.hpp"
#include "../paths/paths.hpp"
#include "../archive/archive.hpp"
#include "../build/builder.hpp"
#include "../store/store.hpp"
#include "../activation/activation.hpp"

#include <iostream>
#include <stdexcept>


namespace athena::install {

void install_package(
    const std::filesystem::path& package_file,
    const std::filesystem::path& target_root
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

    /*
     * Create a dedicated build directory for this package.
     *
     * Build artifacts must be kept separate from the source tree.
     * This is especially important for out-of-source build systems
     * such as CMake.
     */
    const std::filesystem::path build_directory =
        athena::paths::build() /
        (package.name + "-" + package.version);

    /*
     * The staging directory is kept separate from the build
     * directory. It represents the temporary filesystem root
     * produced by the package installation step.
     */
    const std::filesystem::path staging =
        athena::paths::build() /
        (package.name + "-" + package.version + "-install");

    std::cout
        << "Directory di build: "
        << build_directory << '\n';

    std::cout
        << "Compilazione...\n";

    athena::build::build(
        extracted,
        build_directory,
        package.build_system
    );

    std::cout
        << "Compilazione completata.\n";

    std::cout
        << "Installazione nello staging: "
        << staging << '\n';

    athena::build::install(
        extracted,
        build_directory,
        staging,
        package.build_system
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
            package.build_system
        );

    std::cout
        << "Installazione nello store completata: "
        << installed << '\n';

    std::cout
        << "Attivazione del pacchetto...\n";

    athena::activation::activate(
        installed,
        target_root / "usr"
    );

    std::cout
        << "Attivazione completata.\n";
}

}
