#include "engine.hpp"
#include "settings.hpp"
#include "files/settings_io.hpp"
#include "files/engine_paths.hpp"
#include "util/platform.hpp"
#include "util/command_line.hpp"
#include "debug/Logger.hpp"

#include <stdexcept>

static debug::Logger logger("main");

int main(int argc, char** argv) {
    debug::Logger::init("latest.log");
    logger.info() << "initialized";

    EnginePaths paths;
    if (!parse_cmdline(argc, argv, paths))
        return EXIT_SUCCESS;

    platform::configure_encoding();
    try {
        EngineSettings settings;
        SettingsHandler handler(settings);
        
        Engine engine(settings, handler, &paths);

        engine.mainloop();
    }
    catch (const initialize_error& err) {
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
