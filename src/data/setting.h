#ifndef DATA_SETTING_H_
#define DATA_SETTING_H_

#include <string>

enum class setting_format {
    simple, percent
};

class Setting {
protected:
    setting_format format;
public:
    Setting(setting_format format) : format(format) {
    }

    virtual ~Setting() {}

    virtual setting_format getFormat() const {
        return format;
    }

    virtual std::string toString() const = 0;
};

template<class T>
class NumberSetting : public Setting {
protected:
    T value;
    T min;
    T max;
public:
    NumberSetting(T value, T min, T max, setting_format format)
    : Setting(format), value(value), min(min), max(max) {}

    T& operator*() {
        return value;
    }

    T get() const {
        return value;
    }

    void set(T value) {
        this->value = value;
    }

    T getMin() const {
        return min;
    }

    T getMax() const {
        return max;
    }

    T getT() const {
        return (value - min) / (max - min);
    }

    virtual std::string toString() const override;

    static inline NumberSetting createPercent(T def) {
        return NumberSetting(def, 0.0, 1.0, setting_format::percent);
    }
};

#endif // DATA_SETTING_H_
