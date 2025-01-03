#include "Screen.hpp"

#include "graphics/core/Batch2D.hpp"
#include "engine/Engine.hpp"

Screen::Screen(Engine& engine)
  : engine(engine), 
    batch(std::make_unique<Batch2D>(1024)) {
}

Screen::~Screen() {
}
