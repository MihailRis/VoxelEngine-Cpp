#include "engine/Engine.hpp"
#include "util/platform.hpp"
#include "util/command_line.hpp"
#include "debug/Logger.hpp"

#include <iostream>
#include <stdexcept>

static debug::Logger logger("main");

int main(int argc, char** argv) {
    CoreParameters coreParameters;
    try {
        if (!parse_cmdline(argc, argv, coreParameters)) {
            return EXIT_SUCCESS;
        }
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    debug::Logger::init(coreParameters.userFolder.string()+"/latest.log");
    platform::configure_encoding();
    try {
        Engine(std::move(coreParameters)).run();
    } catch (const initialize_error& err) {
        logger.error() << "could not to initialize engine\n" << err.what();
    }
#if defined(NDEBUG) and defined(_WIN32)
    catch (const std::exception& err) {
        logger.error() << "uncaught exception: " << err.what();
        debug::Logger::flush();
        throw;
    }
#endif
    return EXIT_SUCCESS;
}
