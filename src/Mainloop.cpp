#include "Mainloop.hpp"

#include "debug/Logger.hpp"
#include "engine.hpp"
#include "frontend/screens/MenuScreen.hpp"
#include "frontend/screens/LevelScreen.hpp"
#include "window/Window.hpp"
#include "world/Level.hpp"

static debug::Logger logger("mainloop");

Mainloop::Mainloop(Engine& engine) : engine(engine) {
}

void Mainloop::run() {
    auto& time = engine.getTime();

    engine.setLevelConsumer([this](auto level) {
        if (level == nullptr) {
            // destroy LevelScreen and run quit callbacks
            engine.setScreen(nullptr);
            // create and go to menu screen
            engine.setScreen(std::make_shared<MenuScreen>(engine));
        } else {
            engine.setScreen(std::make_shared<LevelScreen>(engine, std::move(level)));
        }
    });

    logger.info() << "starting menu screen";
    engine.setScreen(std::make_shared<MenuScreen>(engine));
    
    logger.info() << "main loop started";
    while (!Window::isShouldClose()){
        time.update(Window::time());
        engine.updateFrontend();
        if (!Window::isIconified()) {
            engine.renderFrame();
        }
        engine.postUpdate();
        engine.nextFrame();
    }
    logger.info() << "main loop stopped";
}
