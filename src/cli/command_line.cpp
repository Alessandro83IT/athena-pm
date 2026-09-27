#include "command_line.hpp"

namespace athena::cli {

/*
 * Parse the raw command-line arguments received by main().
 *
 * Athena uses a simple command-oriented syntax:
 *
 *     athena <command> [arguments...]
 *
 * The first argument after the executable name is therefore treated
 * as the command, while all following arguments are stored separately
 * for the command implementation to interpret.
 */
CommandLine parse(
    int argc,
    char* argv[]
)
{
    CommandLine result;

    /*
     * When no command was supplied, return an empty CommandLine.
     *
     * The higher-level CLI layer is responsible for deciding how
     * an empty command should be presented to the user.
     */
    if (argc < 2) {
        return result;
    }

    /*
     * argv[0] contains the executable name.
     *
     * argv[1] is the first actual argument and therefore represents
     * the Athena command.
     */
    result.command = argv[1];

    /*
     * Every argument after the command belongs to the command itself.
     *
     * For example:
     *
     *     athena install hello
     *
     * produces:
     *
     *     command   = "install"
     *     arguments = ["hello"]
     */
    for (int i = 2; i < argc; ++i) {
        result.arguments.emplace_back(argv[i]);
    }

    return result;
}

}
