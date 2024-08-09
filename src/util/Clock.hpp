#pragma once

namespace util {
    class Clock {
        int tickRate;
        int tickParts;

        float tickTimer = 0.0f;
        int tickId = 0;
        int tickPartsUndone = 0;
    public:
        Clock(int tickRate, int tickParts);

        bool update(float delta);

        int getParts() const;
        int getPart() const;
        int getTickRate() const;
        int getTickId() const;
    };
}
