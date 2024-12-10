#include "TestMainloop.hpp"

#include "logic/scripting/scripting.hpp"
#include "logic/LevelController.hpp"
#include "interfaces/Process.hpp"
#include "debug/Logger.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"
#include "engine.hpp"

static debug::Logger logger("mainloop");

inline constexpr int TPS = 20;

TestMainloop::TestMainloop(Engine& engine) : engine(engine) {
}

TestMainloop::~TestMainloop() = default;

void TestMainloop::run() {
    const auto& coreParams = engine.getCoreParameters();
    auto& time = engine.getTime();

    if (coreParams.testFile.empty()) {
        logger.info() << "nothing to do";
        return;
    }
    engine.setLevelConsumer([this](auto level) {
        setLevel(std::move(level));
    });

    logger.info() << "starting test " << coreParams.testFile;
    auto process = scripting::start_coroutine(coreParams.testFile);
    while (process->isActive()) {
        time.step(1.0f / static_cast<float>(TPS));
        process->update();
        if (controller) {
            float delta = time.getDelta();
            controller->getLevel()->getWorld()->updateTimers(delta);
            controller->update(glm::min(delta, 0.2f), false, false);
        }
    }
    logger.info() << "test finished";
}

void TestMainloop::setLevel(std::unique_ptr<Level> level) {
    if (level == nullptr) {
        controller->onWorldQuit();
        engine.getPaths()->setCurrentWorldFolder(fs::path());
        controller = nullptr;
    } else {
        controller = std::make_unique<LevelController>(&engine, std::move(level));
    }
}
