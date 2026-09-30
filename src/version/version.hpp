#pragma once

#include <string>

namespace athena::version {

bool less(
    const std::string& left,
    const std::string& right
);

bool greater(
    const std::string& left,
    const std::string& right
);

}
