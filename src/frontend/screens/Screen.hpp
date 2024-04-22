#ifndef FRONTEND_SCREENS_SCREEN_HPP_
#define FRONTEND_SCREENS_SCREEN_HPP_

#include "../../util/ObjectsKeeper.hpp"

class Engine;
class Batch2D;

/// @brief Screen is a mainloop state
class Screen : public util::ObjectsKeeper {
protected:
    Engine* engine;
    std::unique_ptr<Batch2D> batch;
public:
    Screen(Engine* engine);
    virtual ~Screen();
    virtual void update(float delta) = 0;
    virtual void draw(float delta) = 0;
    virtual void onEngineShutdown() {};
};

#endif // FRONTEND_SCREENS_SCREEN_HPP_
