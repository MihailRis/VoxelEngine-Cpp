#pragma once

#include <vector>
#include <stdexcept>
#include <functional>
#include <glm/glm.hpp>

namespace util {
    template<class T>
    using OutCallback = std::function<void(const T&)>;

    template<class T, typename TCoord=int>
    class AreaMap2D {
        glm::vec<2, TCoord> offset;
        glm::vec<2, TCoord> size;
        std::vector<T> firstBuffer;
        std::vector<T> secondBuffer;
        OutCallback<T> outCallback;

        size_t valuesCount = 0;
    
        void translate(const glm::vec<2, TCoord>& delta) {
            if (delta.x == 0 && delta.y == 0) {
                return;
            }
            std::fill(secondBuffer.begin(), secondBuffer.end(), T{});
            for (TCoord y = 0; y < size.y; y++) {
                for (TCoord x = 0; x < size.x; x++) {
                    auto& value = firstBuffer[y * size.x + x];
                    auto nx = x - delta.x;
                    auto ny = y - delta.y;
                    if (value == T{}) {
                        continue;
                    }
                    if (nx < 0 || ny < 0 || nx >= size.x || ny >= size.y) {
                        if (outCallback) {
                            outCallback(value);
                        }
                        valuesCount--;
                        continue;
                    }
                    secondBuffer[ny * size.x + nx] = value;
                }
            }
            std::swap(firstBuffer, secondBuffer);
            offset += delta;
        }
    public:
        AreaMap2D(glm::vec<2, TCoord> size)
            : size(size), 
              firstBuffer(size.x * size.y), secondBuffer(size.x * size.y) {
        }

        const T* getIf(const glm::vec<2, TCoord>& pos) const {
            auto localPos = pos - offset;
            if (localPos.x < 0 || localPos.y < 0 || localPos.x >= size.x ||
                localPos.y >= size.y) {
                return nullptr;
            }
            return &firstBuffer[localPos.y * size.x + localPos.x];
        }

        T get(const glm::vec<2, TCoord>& pos) {
            auto localPos = pos - offset;
            if (localPos.x < 0 || localPos.y < 0 || localPos.x >= size.x ||
                localPos.y >= size.y) {
                return T{};
            }
            return firstBuffer[localPos.y * size.x + localPos.x];
        }

        const T& require(const glm::vec<2, TCoord>& pos) const {
            auto localPos = pos - offset;
            if (localPos.x < 0 || localPos.y < 0 || localPos.x >= size.x ||
                localPos.y >= size.y) {
                throw std::invalid_argument("position is out of window");
            }
            return firstBuffer[localPos.y * size.x + localPos.x];
        }

        bool set(const glm::vec<2, TCoord>& pos, T value) {
            auto localPos = pos - offset;
            if (localPos.x < 0 || localPos.y < 0 || localPos.x >= size.x ||
                localPos.y >= size.y) {
                return false;
            }
            auto& element = firstBuffer[localPos.y * size.x + localPos.x];
            if (!element) {
                valuesCount++;
            }
            element = std::move(value);
            return true;
        }

        void setOutCallback(const OutCallback<T>& callback) {
            outCallback = callback;
        }

        void resize(const glm::vec<2, TCoord>& newSize) {
            if (newSize.x < size.x) {
                TCoord delta = size.x - newSize.x;
                translate({delta / 2, 0});
                translate({-delta, 0});
                translate({delta, 0});
            }
            if (newSize.y < size.y) {
                TCoord delta = size.y - newSize.y;
                translate({0, delta / 2});
                translate({0, -delta});
                translate({0, delta});
            }
            const TCoord newVolume = newSize.x * newSize.y;
            std::vector<T> newFirstBuffer(newVolume);
            std::vector<T> newSecondBuffer(newVolume);
            for (TCoord y = 0; y < size.y && y < newSize.y; y++) {
                for (TCoord x = 0; x < size.x && x < newSize.x; x++) {
                    newFirstBuffer[y * newSize.x + x] = firstBuffer[y * size.x + x];
                }
            }
            size = newSize;
            firstBuffer = std::move(newFirstBuffer);
            secondBuffer = std::move(newSecondBuffer);
        }

        void setCenter(const glm::vec<2, TCoord>& center) {
            auto delta = center - (offset + size / 2);
            if (delta.x | delta.y) {
                translate({delta.x, delta.y});
            }
        }

        void clear() {
            for (TCoord i = 0; i < size.x * size.y; i++) {
                auto value = firstBuffer[i];
                firstBuffer[i] = {};
                if (outCallback) {
                    outCallback(value);
                }
            }
            valuesCount = 0;
        }

        const glm::vec<2, TCoord>& getOffset() const {
            return offset;
        }

        const glm::vec<2, TCoord>& getSize() const {
            return size;
        }

        const std::vector<T>& getBuffer() const {
            return firstBuffer;
        }

        size_t count() const {
            return valuesCount;
        }

        TCoord area() const {
            return size.x * size.y;
        }
    };
}
