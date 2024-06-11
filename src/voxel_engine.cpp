#include "engine.hpp"
#include "settings.hpp"
#include "files/settings_io.hpp"
#include "files/engine_paths.hpp"
#include "util/platform.hpp"
#include "util/command_line.hpp"
#include "debug/Logger.hpp"

#include <stdexcept>

static debug::Logger logger("main");

#include <entt/entt.hpp>

struct position {
    float x;
    float y;
};

struct velocity {
    float dx;
    float dy;
};

void update(entt::registry &registry) {
    auto view = registry.view<const position, velocity>();

    view.each([](const auto &pos, const auto &vel) {
        logger.info() << pos.x << "," << pos.y << " - " << vel.dx << ", " << vel.dy;
    });
}

int entt_test() {
    entt::registry registry;

    for(auto i = 0u; i < 10u; ++i) {
        const auto entity = registry.create();
        registry.emplace<position>(entity, i * 1.f, i * 1.f);
        if(i % 2 == 0) { registry.emplace<velocity>(entity, i * .1f, i * .1f); }
    }

    update(registry);
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    debug::Logger::init("latest.log");

    return entt_test();

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
