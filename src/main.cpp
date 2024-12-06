#include "engine.hpp"
#include "util/platform.hpp"
#include "util/command_line.hpp"
#include "debug/Logger.hpp"

#include <stdexcept>

static debug::Logger logger("main");

int main(int argc, char** argv) {
    debug::Logger::init("latest.log");

    CoreParameters coreParameters;
    if (!parse_cmdline(argc, argv, coreParameters)) {
        return EXIT_SUCCESS;
    }
    platform::configure_encoding();
    try {
        Engine(std::move(coreParameters)).run();
    } catch (const initialize_error& err) {
        logger.error() << "could not to initialize engine\n" << err.what();
    }
#ifdef NDEBUG
    catch (const std::exception& err) {
        logger.error() << "uncaught exception: " << err.what();
        debug::Logger::flush();
        throw;
    }
#endif
    return EXIT_SUCCESS;
}
