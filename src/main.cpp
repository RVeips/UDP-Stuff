#include "Log.hpp"
#include <argparse/argparse.hpp>

int main(int argc, char **argv) {
    const auto version_string = std::to_string(CFXS_BUILD_VERSION_MAJOR) + "." + std::to_string(CFXS_BUILD_VERSION_MINOR);
    argparse::ArgumentParser args("cfxs-build", version_string);

    try {
        args.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        Log.error("{}", err.what());
        std::cout << args.usage() << std::endl;
        return 1;
    }

    // try {
    //     if (args["-t"] == true) {
    //         s_log_trace = true;
    //     }
    // } catch (const std::exception &e) {
    // }
    initialize_logging();

    Log.info("UDP Stuff v{}", version_string);

    return 0;
}
