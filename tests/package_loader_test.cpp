#include "../src/package/package_loader.hpp"

#include <cassert>
#include <iostream>

int main()
{
    const auto package =
        athena::package::load_from_file(
            "packages/cmake-test/package.toml"
        );

    assert(package.name == "cmake-test");
    assert(package.version == "1.1");

    assert(package.dependencies.size() == 1);
    assert(package.dependencies[0] == "test-dependency");

    std::cout
        << "Package loader dependency test passed.\n";

    return 0;
}
