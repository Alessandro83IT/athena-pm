#include "../src/generation/generation.hpp"
#include "../src/store/gc.hpp"
#include "../src/store/manifest.hpp"
#include "../src/store/metadata.hpp"

#include <cassert>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
    const auto test_root =
        std::filesystem::path(
            "/tmp/athena-gc-test"
        );

    std::filesystem::remove_all(
        test_root
    );

    const auto state_directory =
        test_root / "state";

    const auto store_directory =
        test_root / "store";

    std::filesystem::create_directories(
        store_directory
    );

    /*
     * Create three valid Athena store entries:
     *
     *     kept       -> referenced by a generation
     *     historical -> referenced only by an old generation
     *     orphan     -> referenced by no generation
     */
    const auto kept =
        store_directory / "package-kept-1.0";

    const auto historical =
        store_directory / "package-historical-1.0";

    const auto orphan =
        store_directory / "package-orphan-1.0";

    for (const auto& store : {
        kept,
        historical,
        orphan
    }) {
        std::filesystem::create_directories(
            store / ".athena"
        );

        athena::store::write_metadata(
            store,
            {
                "test-package",
                "1.0",
                "test-source",
                "test-sha256",
                "test"
            }
        );
    }

    /*
     * Also create an unrelated directory. The GC must never remove
     * objects that are not recognized as Athena store entries.
     */
    const auto unrelated =
        store_directory / "unrelated";

    std::filesystem::create_directories(
        unrelated
    );

    /*
     * Generation 0 references the historical store.
     */
    athena::generation::GenerationManager manager(
        state_directory
    );

    manager.initialize();

    manager.create(
        {
            {
                "package-historical",
                historical.string()
            }
        }
    );

    /*
     * Generation 2 references the current store.
     *
     * The historical store must survive because generation 1 still
     * makes it reachable for rollback.
     */
    manager.create(
        {
            {
                "package-kept",
                kept.string()
            }
        }
    );

    athena::store::GarbageCollector gc(
        store_directory,
        state_directory
    );

    const auto removed =
        gc.collect();

    /*
     * The orphan is unreachable and must be collected.
     */
    assert(
        !std::filesystem::exists(orphan)
    );

    assert(
        std::find(
            removed.begin(),
            removed.end(),
            orphan
        ) != removed.end()
    );

    /*
     * Both current and historical stores remain available.
     */
    assert(
        std::filesystem::exists(kept)
    );

    assert(
        std::filesystem::exists(historical)
    );

    /*
     * Unrelated directories are outside the GC ownership boundary.
     */
    assert(
        std::filesystem::exists(unrelated)
    );

    /*
     * A second collection is idempotent: there is nothing else to
     * remove.
     */
    const auto removed_again =
        gc.collect();

    assert(
        removed_again.empty()
    );

    /*
     * A symlink in the store root is not an Athena store entry and
     * must never be followed or removed by the collector.
     */
    const auto external_directory =
        test_root / "external";

    std::filesystem::create_directories(
        external_directory
    );

    const auto external_file =
        external_directory / "important";

    {
        std::ofstream file(external_file);
        assert(file);
        file << "must survive\n";
    }

    const auto store_symlink =
        store_directory / "external-link";

    std::filesystem::create_directory_symlink(
        external_directory,
        store_symlink
    );

    const auto removed_with_symlink =
        gc.collect();

    assert(
        removed_with_symlink.empty()
    );

    assert(
        std::filesystem::is_symlink(
            store_symlink
        )
    );

    assert(
        std::filesystem::exists(
            external_file
        )
    );

    /*
     * A generation referencing a path outside the configured store
     * root must be rejected rather than allowing the GC to treat that
     * path as an Athena-owned store entry.
     */
    const auto invalid_generation =
        manager.create(
            {
                {
                    "external-package",
                    external_directory.string()
                }
            }
        );

    (void)invalid_generation;

    bool invalid_path_rejected = false;

    try {
        gc.collect();
    }
    catch (const std::runtime_error&) {
        invalid_path_rejected = true;
    }

    assert(
        invalid_path_rejected
    );

    std::filesystem::remove_all(
        test_root
    );

    std::cout
        << "Garbage collector tests passed."
        << '\n';

    return 0;
}
