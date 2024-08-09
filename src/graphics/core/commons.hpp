#pragma once

enum class DrawPrimitive {
    point = 0,
    line,
    triangle,
};

enum class BlendMode {
    normal, addition, inversion
};

class Flushable {
public:
    virtual ~Flushable() = default;

    virtual void flush() = 0;
};
