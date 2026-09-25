#include "command_line.hpp"

namespace athena::cli {

CommandLine parse(int argc, char* argv[])
{
    CommandLine result;

    if (argc < 2) {
        return result;
    }

    result.command = argv[1];

    for (int i = 2; i < argc; ++i) {
        result.arguments.emplace_back(argv[i]);
    }

    return result;
}

}
