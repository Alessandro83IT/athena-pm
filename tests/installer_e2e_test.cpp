#include "../src/install/installer.hpp"
#include "../src/paths/paths.hpp"
#include "../src/store/store.hpp"
#include "../src/generation/generation.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::string sha256(const std::filesystem::path& file)
{
    /*
     * Use the same host utility used by Athena's verifier to obtain
     * the checksum of the locally generated fixture archive.
     */
    const std::string command =
        "sha256sum \"" + file.string() + "\"";

    FILE* pipe = popen(command.c_str(), "r");

    if (pipe == nullptr) {
        throw std::runtime_error(
            "Impossibile eseguire sha256sum"
        );
    }

    char buffer[256]{};
    std::string output;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }

    const int status = pclose(pipe);

    if (status != 0) {
        throw std::runtime_error(
            "sha256sum ha restituito un errore"
        );
    }

    const auto separator = output.find(' ');

    if (separator == std::string::npos) {
        throw std::runtime_error(
            "Output inatteso da sha256sum"
        );
    }

    return output.substr(0, separator);
}

void create_fixture(
    const std::filesystem::path& fixture_root
)
{
    const auto source =
        fixture_root / "athena-e2e-package-1.0";

    std::filesystem::create_directories(
        source / "usr/share/athena-e2e"
    );

    /*
     * This is deliberately a tiny real CMake package. The test
     * therefore exercises Athena's actual CMake build backend rather
     * than mocking the compilation/install stages.
     */
    {
        std::ofstream cmake(source / "CMakeLists.txt");

        cmake
            << "cmake_minimum_required(VERSION 3.20)\n"
            << "project(athena_e2e_package VERSION 1.0 LANGUAGES NONE)\n"
            << "install(\n"
            << "    FILES \"${CMAKE_CURRENT_SOURCE_DIR}/usr/share/athena-e2e/hello.txt\"\n"
            << "    DESTINATION share/athena-e2e\n"
            << ")\n";
    }

    {
        std::ofstream hello(
            source / "usr/share/athena-e2e/hello.txt"
        );

        hello << "Athena end-to-end test\n";
    }

    /*
     * The archive must contain exactly one top-level directory,
     * matching the contract enforced by athena::archive::extract().
     */
    const auto archive =
        fixture_root / "athena-e2e-package-1.0.tar.gz";

    const std::string command =
        "tar -czf \"" + archive.string() +
        "\" -C \"" + fixture_root.string() +
        "\" athena-e2e-package-1.0";

    if (std::system(command.c_str()) != 0) {
        throw std::runtime_error(
            "Creazione dell'archivio fixture fallita"
        );
    }
}

}

int main()
{
    const auto test_root =
        std::filesystem::temp_directory_path() /
        "athena-installer-e2e-test";

    std::filesystem::remove_all(test_root);
    std::filesystem::create_directories(test_root);

    /*
     * Redirect every Athena-managed path into the temporary test
     * hierarchy. No production path such as /var/lib/athena is used.
     */
    athena::paths::set_root(test_root / "athena");

    try {
        const auto fixture_root =
            test_root / "fixture";

        std::filesystem::create_directories(fixture_root);

        create_fixture(fixture_root);

        const auto archive =
            fixture_root / "athena-e2e-package-1.0.tar.gz";

        const auto checksum = sha256(archive);

        athena::package::Package package;
        package.name = "athena-e2e-package";
        package.version = "1.0";
        package.description = "End-to-end installer test package";
        package.source = "file://" + archive.string();
        package.sha256 = checksum;
        package.build_system = "cmake";

        /*
         * A real InstallPlan is passed to the real installer. No
         * installation step is mocked or bypassed.
         */
        athena::install::InstallPlan plan;
        plan.add(package);

        const auto target_root =
            test_root / "active";

        athena::install::install_plan(
            plan,
            target_root
        );

        /*
         * Verify that the immutable store contains the package.
         */
        const auto store_path =
            athena::store::find(package.name);

        assert(!store_path.empty());
        assert(std::filesystem::is_directory(store_path));

        /*
         * Verify that the generated environment points to the store
         * entry and that the generation has become current.
         */
        athena::generation::GenerationManager generations(
            athena::paths::root()
        );

        const auto current = generations.current();

        assert(current.entries.size() == 1);
        assert(current.entries[0].package == package.name);
        assert(
            std::filesystem::path(
                current.entries[0].store_path
            ) == store_path
        );

        /*
         * The activation layer represents package files through
         * symlinks into the immutable store.
         */
        const auto activated_file =
            target_root /
            "share/athena-e2e/hello.txt";

        assert(std::filesystem::is_symlink(activated_file));

        assert(
            std::filesystem::read_symlink(activated_file) ==
            store_path / "share/athena-e2e/hello.txt"
        );

        std::filesystem::remove_all(test_root);
        athena::paths::reset_root();

        return 0;
    }
    catch (...) {
        /*
         * Always restore the global path override before propagating
         * the failure so the test cannot contaminate later tests.
         */
        std::filesystem::remove_all(test_root);
        athena::paths::reset_root();
        throw;
    }
}
