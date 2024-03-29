#ifndef DATA_SETTING_H_
#define DATA_SETTING_H_

#include <string>

enum class setting_format {
    simple, percent
};

template<class T>
class Setting {
protected:
    T value;
    setting_format format;
public:
    Setting(T value, setting_format format) : value(value), format(format) {
    }

    virtual ~Setting() {}

    T& operator*() {
        return value;
    }

    virtual const T& get() const {
        return value;
    }

    virtual void set(const T& value) {
        this->value = value;
    }

    virtual setting_format getFormat() const {
        return format;
    }

    virtual std::string toString() const = 0;
};

class NumberSetting : public Setting<double> {
protected:
    double min;
    double max;
public:
    NumberSetting(double value, double min, double max, setting_format format)
    : Setting(value, format), min(min), max(max) {}

    double& operator*() {
        return value;
    }

    double getMin() const {
        return min;
    }

    double getMax() const {
        return max;
    }

    double getT() const {
        return (value - min) / (max - min);
    }

    virtual std::string toString() const override;

    static inline NumberSetting createPercent(double def) {
        return NumberSetting(def, 0.0, 1.0, setting_format::percent);
    }
};

#endif // DATA_SETTING_H_
