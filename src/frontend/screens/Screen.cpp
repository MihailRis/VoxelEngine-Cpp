#include "Screen.hpp"

#include "../../graphics/core/Batch2D.hpp"
#include "../../engine.h"

Screen::Screen(Engine* engine) : engine(engine), batch(new Batch2D(1024)) {
}

Screen::~Screen() {
}