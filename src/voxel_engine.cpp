#include <stdexcept>

#include "core_defs.h"
#include "engine.h"
#include "files/files.h"
#include "files/settings_io.hpp"
#include "files/engine_paths.h"
#include "util/platform.h"
#include "util/command_line.hpp"
#include "window/Events.hpp"
#include "debug/Logger.hpp"

static debug::Logger logger("main");

int main(int argc, char** argv) {
    debug::Logger::init("latest.log");

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
