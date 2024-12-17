#include "ServerMainloop.hpp"

#include "logic/scripting/scripting.hpp"
#include "logic/LevelController.hpp"
#include "interfaces/Process.hpp"
#include "debug/Logger.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"
#include "util/platform.hpp"
#include "engine.hpp"

#include <chrono>

using namespace std::chrono;

static debug::Logger logger("mainloop");

inline constexpr int TPS = 20;

ServerMainloop::ServerMainloop(Engine& engine) : engine(engine) {
}

ServerMainloop::~ServerMainloop() = default;

void ServerMainloop::run() {
    const auto& coreParams = engine.getCoreParameters();
    auto& time = engine.getTime();

    if (coreParams.scriptFile.empty()) {
        logger.info() << "nothing to do";
        return;
    }
    engine.setLevelConsumer([this](auto level) {
        setLevel(std::move(level));
    });

    logger.info() << "starting test " << coreParams.scriptFile;
    auto process = scripting::start_coroutine(coreParams.scriptFile);

    double targetDelta = 1.0f / static_cast<float>(TPS);
    double delta = targetDelta;
    auto begin = steady_clock::now();
    while (process->isActive()) {
        if (engine.isQuitSignal()) {
            process->terminate();
            logger.info() << "script has been terminated due to quit signal";
            break;
        }
        time.step(delta);
        process->update();
        if (controller) {
            float delta = time.getDelta();
            controller->getLevel()->getWorld()->updateTimers(delta);
            controller->update(glm::min(delta, 0.2f), false);
        }

        if (!coreParams.testMode) {
            auto end = steady_clock::now();
            platform::sleep(targetDelta * 1000 - 
                duration_cast<microseconds>(end - begin).count() / 1000);
            end = steady_clock::now();
            delta = duration_cast<microseconds>(end - begin).count() / 1e6;
            begin = end;
        }
    }
    logger.info() << "test finished";
}

void ServerMainloop::setLevel(std::unique_ptr<Level> level) {
    if (level == nullptr) {
        controller->onWorldQuit();
        engine.getPaths()->setCurrentWorldFolder(fs::path());
        controller = nullptr;
    } else {
        controller = std::make_unique<LevelController>(
            &engine, std::move(level), nullptr
        );
    }
}
