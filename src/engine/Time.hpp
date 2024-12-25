#pragma once

#include <stdint.h>

class Time {
    uint64_t frame = 0;
    double lastTime = 0.0;
    double delta = 0.0;
public:
    Time() {}

    void update(double currentTime) {
        frame++;
        delta = currentTime - lastTime;
        lastTime = currentTime;
    }

    void step(double delta) {
        frame++;
        lastTime += delta;
        this->delta = delta;
    }

    void set(double currentTime) {
        lastTime = currentTime;
    }

    double getDelta() const {
        return delta;
    }

    double getTime() const {
        return lastTime;
    }
};
