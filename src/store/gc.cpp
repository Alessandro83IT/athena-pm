#include "gc.hpp"

#include "../generation/generation.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace athena::store {

/*
 * Store entries are identified by their canonical absolute paths.
 *
 * Generations may contain equivalent paths written with different
 * relative components, so normalizing both sides prevents an entry
 * from being collected merely because its textual path differs.
 */
static std::filesystem::path normalize_path(
    const std::filesystem::path& path
)
{
    return std::filesystem::absolute(path).lexically_normal();
}

GarbageCollector::GarbageCollector(
    std::filesystem::path store_directory,
    std::filesystem::path state_directory
)
    : store_directory_(std::move(store_directory)),
      state_directory_(std::move(state_directory))
{
}

/*
 * A package store entry contains Athena metadata below .athena.
 *
 * This deliberately ignores unrelated directories in the store so
 * that the collector does not delete files it does not own.
 */
bool GarbageCollector::is_store_entry(
    const std::filesystem::path& path
)
{
    /*
     * Symlinks are never considered Athena store entries. The GC must
     * only delete directories physically owned by the store root and
     * must never follow a link to another filesystem location.
     */
    if (std::filesystem::is_symlink(path)) {
        return false;
    }

    return
        std::filesystem::is_directory(path) &&
        std::filesystem::is_regular_file(
            path / ".athena" / "metadata.toml"
        );
}

/*
 * Collect all store paths referenced by every persisted generation.
 *
 * Historical generations are intentionally included. Removing their
 * stores would make rollback impossible.
 */
std::vector<std::filesystem::path>
GarbageCollector::referenced_stores() const
{
    athena::generation::GenerationManager manager(
        state_directory_
    );

    const auto generations =
        manager.list();

    const auto normalized_store_root =
        normalize_path(store_directory_);

    std::unordered_set<std::string> unique_paths;

    for (const auto& generation : generations) {

        for (const auto& entry : generation.entries) {

            const auto normalized =
                normalize_path(entry.store_path);

            /*
             * A generation may protect only an entry below the
             * configured Athena store root. This prevents malformed
             * generation metadata from causing the GC to treat an
             * arbitrary filesystem path as part of the store.
             */
            const auto relative =
                normalized.lexically_relative(
                    normalized_store_root
                );

            if (
                relative.empty() ||
                relative == "." ||
                relative.string().starts_with("..") ||
                relative.is_absolute()
            ) {
                throw std::runtime_error(
                    "Store path fuori dallo store Athena: " +
                    entry.store_path
                );
            }

            unique_paths.insert(
                normalized.string()
            );
        }
    }

    std::vector<std::filesystem::path> result;

    result.reserve(
        unique_paths.size()
    );

    for (const auto& path : unique_paths) {
        result.emplace_back(path);
    }

    return result;
}

/*
 * Remove unreachable store entries.
 *
 * Only directories owned by Athena are considered. Unrelated files
 * or directories under the store root are left untouched.
 */
std::vector<std::filesystem::path>
GarbageCollector::collect()
{
    if (!std::filesystem::exists(store_directory_)) {
        return {};
    }

    if (!std::filesystem::is_directory(store_directory_)) {
        throw std::runtime_error(
            "Lo store non è una directory: " +
            store_directory_.string()
        );
    }

    const auto referenced =
        referenced_stores();

    std::unordered_set<std::string> referenced_set;

    for (const auto& path : referenced) {
        referenced_set.insert(
            normalize_path(path).string()
        );
    }

    std::vector<std::filesystem::path> removed;

    for (
        const auto& entry :
        std::filesystem::directory_iterator(
            store_directory_
        )
    ) {
        if (!is_store_entry(entry.path())) {
            continue;
        }

        const auto normalized =
            normalize_path(entry.path());

        if (referenced_set.contains(
                normalized.string()
            )) {
            continue;
        }

        std::filesystem::remove_all(
            entry.path()
        );

        removed.push_back(
            entry.path()
        );
    }

    std::sort(
        removed.begin(),
        removed.end()
    );

    return removed;
}

}
