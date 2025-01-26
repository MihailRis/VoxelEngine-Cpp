#pragma once

#include <limits>
#include <glm/glm.hpp>

namespace util {
    template<int N, typename T, bool angular=false>
    class VecInterpolation {
        bool enabled;
        glm::vec<N, T> prevPos {std::numeric_limits<T>::quiet_NaN()};
        glm::vec<N, T> nextPos {};
        T refreshInterval = 0.0;
        T timer = 0.0;
        T intervalUpdateFactor = 0.1;
    public:
        VecInterpolation(bool enabled) : enabled(enabled) {}

        void refresh(const glm::vec<N, T>& position) {
            auto current = getCurrent();
            prevPos = current;
            nextPos = position;
            refreshInterval = timer * intervalUpdateFactor +
                              refreshInterval * (1.0 - intervalUpdateFactor);
            timer = 0.0;

            if constexpr (angular) {
                for (int i = 0; i < N; i++) {
                    T diff = nextPos[i] - prevPos[i];
                    if (glm::abs(diff) > 180.0f) {
                        nextPos[i] += (diff > 0.0f ? -360.0f : 360.0f);
                    }
                }
            }
        }

        void updateTimer(T delta) {
            timer += delta;
        }

        glm::vec<N, T> getCurrent() const {
            if (refreshInterval < 0.001 || std::isnan(prevPos.x)) {
                return nextPos;
            }
            T t = timer / refreshInterval;
            return nextPos * t + prevPos * (1.0f - t);
        }

        T getRefreshInterval() const {
            return refreshInterval;
        }

        bool isEnabled() const {
            return enabled;
        }

        void setEnabled(bool enabled) {
            this->enabled = enabled;
        }
    };
}
