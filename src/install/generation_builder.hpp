#pragma once

#include "../generation/generation.hpp"
#include "../package/package.hpp"

#include <filesystem>
#include <utility>
#include <vector>

namespace athena::install {

/*
 * Apply the packages produced by an InstallPlan to the current
 * generation.
 *
 * Packages with the same logical name replace the existing entry.
 * Packages not mentioned by the plan remain active. New package
 * names are appended to the generation.
 *
 * This function only constructs generation metadata. It does not
 * activate files and does not modify persistent generation state.
 */
std::vector<athena::generation::GenerationEntry> build_target_entries(
    const athena::generation::Generation& current,
    const std::vector<
        std::pair<
            athena::package::Package,
            std::filesystem::path
        >
    >& installed
);

}
