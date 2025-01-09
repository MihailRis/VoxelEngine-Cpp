#pragma once

/// @brief Process interface.
class Process {
public:
    virtual ~Process() {}

    virtual bool isActive() const = 0;
    virtual void update() = 0;
    virtual void waitForEnd() = 0;
    virtual void terminate() = 0;
};
