#pragma once

#include <array>
#include <stdexcept>

namespace util {
    template<typename T, int diameter, typename CoordT = int>
    class CentredMatrix {
    public:
        static constexpr CoordT radius = diameter / 2;

        CentredMatrix() {}

        void setCenter(CoordT x, CoordT y) {
            centerX = x;
            centerY = y;
        }

        T& at(CoordT x, CoordT y) {
            x -= centerX - (diameter - radius);
            y -= centerY - (diameter - radius);
            if (x < 0 || y < 0 || x >= diameter || y >= diameter) {
                throw std::invalid_argument("position is out if matrix");
            }
            return arr.at(y * diameter + x);
        }

        auto begin() {
            return arr.begin();
        }

        auto end() {
            return arr.end();
        }

        CoordT beginX() const {
            return centerX - (diameter - radius);
        }

        CoordT beginY() const {
            return centerY - (diameter - radius);
        }

        CoordT endX() const {
            return centerX + radius;
        }
        
        CoordT endY() const {
            return centerY + radius;
        }
    private:
        std::array<T, diameter * diameter> arr;
        CoordT centerX = 0, centerY = 0;
    };
}
