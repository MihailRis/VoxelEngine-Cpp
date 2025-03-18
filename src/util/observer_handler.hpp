#pragma once

#include <functional>

class observer_handler {
public:
    observer_handler() = default;

    observer_handler(std::function<void()> destructor)
        : destructor(std::move(destructor)) {
    }

    observer_handler(const observer_handler&) = delete;

    observer_handler(observer_handler&& handler) noexcept
        : destructor(std::move(handler.destructor)) {
    }

    ~observer_handler() {
        if (destructor) {
            destructor();
        }
    }

    bool operator==(std::nullptr_t) const {
        return destructor == nullptr;
    }

    observer_handler& operator=(const observer_handler& handler) = delete;

    observer_handler& operator=(observer_handler&& handler) noexcept {
        if (destructor) {
            destructor();
        }
        destructor = std::move(handler.destructor);
        return *this;
    }
private:
    std::function<void()> destructor;
};
