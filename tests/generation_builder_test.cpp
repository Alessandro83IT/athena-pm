#include "../src/install/generation_builder.hpp"

#include <cassert>
#include <filesystem>
#include <vector>

int main()
{
    const athena::generation::Generation current{
        7,
        {
            {"package-a", "/athena/store/package-a-1.0"},
            {"package-b", "/athena/store/package-b-1.0"},
            {"package-c", "/athena/store/package-c-1.0"}
        }
    };

    athena::package::Package upgraded;
    upgraded.name = "package-b";
    upgraded.version = "2.0";

    athena::package::Package added;
    added.name = "package-d";
    added.version = "1.0";

    const auto result =
        athena::install::build_target_entries(
            current,
            {
                {
                    upgraded,
                    "/athena/store/package-b-2.0"
                },
                {
                    added,
                    "/athena/store/package-d-1.0"
                }
            }
        );

    /*
     * The current generation represents the complete environment.
     * Applying a plan must therefore preserve A and C, replace B,
     * and append the newly introduced D.
     */
    assert(result.size() == 4);

    const athena::generation::GenerationEntry expected_a{
        "package-a",
        "/athena/store/package-a-1.0"
    };

    const athena::generation::GenerationEntry expected_b{
        "package-b",
        "/athena/store/package-b-2.0"
    };

    const athena::generation::GenerationEntry expected_c{
        "package-c",
        "/athena/store/package-c-1.0"
    };

    const athena::generation::GenerationEntry expected_d{
        "package-d",
        "/athena/store/package-d-1.0"
    };

    assert(result[0] == expected_a);
    assert(result[1] == expected_b);
    assert(result[2] == expected_c);
    assert(result[3] == expected_d);

    return 0;
}
