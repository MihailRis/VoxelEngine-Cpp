#ifndef GRAPHICS_CORE_COMMONS_HPP_
#define GRAPHICS_CORE_COMMONS_HPP_

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

#endif // GRAPHICS_CORE_COMMONS_HPP_
