#pragma once

#include "typedefs.hpp"

/// @brief Task is a finite process interface.
/// 'work' is a metric of task progress/remaining work (jobs/bytes/something other)
class Task {
public:
    virtual ~Task() {}

    virtual bool isActive() const = 0;
    virtual uint getWorkTotal() const = 0;
    virtual uint getWorkDone() const = 0;
    virtual void update() = 0;
    virtual void waitForEnd() = 0;
    virtual void terminate() = 0;
};
