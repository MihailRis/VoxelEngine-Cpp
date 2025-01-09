#pragma once

#include <queue>
#include <mutex>
#include "delegates.hpp"

class PostRunnables {
    std::queue<runnable> runnables;
    std::recursive_mutex mutex;
public:
    void postRunnable(runnable task) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        runnables.push(std::move(task));
    }

    void run() {
        std::queue<runnable> tasksToRun;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            std::swap(tasksToRun, runnables);
        }

        while (!tasksToRun.empty()) {
            auto& task = tasksToRun.front();
            task();
            tasksToRun.pop();
        }
    }
};
