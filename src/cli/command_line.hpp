#pragma once

#include <string>
#include <vector>

namespace athena::cli {

struct CommandLine {
    std::string command;
    std::vector<std::string> arguments;
};

CommandLine parse(int argc, char* argv[]);

}
