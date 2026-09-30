#include "../src/dependency/resolver.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using athena::package::Package;

int main()
{
    /*
     * Two versions of the same dependency are available.
     *
     * The resolver must select the newest version.
     */
    Package zlib_old{
        "zlib",
        "1.0",
        "Zlib",
        "",
        "",
        "autotools",
        {}
    };

    Package zlib_new{
        "zlib",
        "2.0",
        "Zlib",
        "",
        "",
        "autotools",
        {}
    };

    Package libfoo{
        "libfoo",
        "1.0",
        "Libfoo",
        "",
        "",
        "autotools",
        {"zlib"}
    };

    Package app{
        "app",
        "1.0",
        "Test application",
        "",
        "",
        "autotools",
        {"libfoo"}
    };

    const std::vector<Package> available{
        zlib_old,
        zlib_new,
        libfoo
    };

    const auto result =
        athena::dependency::resolve(
            app,
            available
        );

    /*
     * Dependency-first ordering:
     *
     *     zlib 2.0
     *     libfoo 1.0
     *     app 1.0
     */
    assert(result.size() == 3);

    assert(result[0].name == "zlib");
    assert(result[0].version == "2.0");

    assert(result[1].name == "libfoo");
    assert(result[1].version == "1.0");

    assert(result[2].name == "app");
    assert(result[2].version == "1.0");

    /*
     * Test missing dependency detection.
     */
    Package broken{
        "broken",
        "1.0",
        "Broken package",
        "",
        "",
        "autotools",
        {"missing"}
    };

    bool missing_dependency_detected = false;

    try {

        athena::dependency::resolve(
            broken,
            available
        );
    }
    catch (const std::runtime_error&) {

        missing_dependency_detected = true;
    }

    assert(missing_dependency_detected);

    /*
     * Test dependency cycle detection.
     */
    Package cycle_a{
        "cycle-a",
        "1.0",
        "Cycle A",
        "",
        "",
        "autotools",
        {"cycle-b"}
    };

    Package cycle_b{
        "cycle-b",
        "1.0",
        "Cycle B",
        "",
        "",
        "autotools",
        {"cycle-a"}
    };

    const std::vector<Package> cyclic_packages{
        cycle_a,
        cycle_b
    };

    bool cycle_detected = false;

    try {

        athena::dependency::resolve(
            cycle_a,
            cyclic_packages
        );
    }
    catch (const std::runtime_error&) {

        cycle_detected = true;
    }

    assert(cycle_detected);

    std::cout
        << "Dependency resolver tests passed.\n";

    return 0;
}
