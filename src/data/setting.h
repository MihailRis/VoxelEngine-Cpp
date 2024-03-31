#ifndef DATA_SETTING_H_
#define DATA_SETTING_H_

#include <limits>
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

    virtual void resetToDefault() = 0;

    virtual setting_format getFormat() const {
        return format;
    }

    virtual std::string toString() const = 0;
};

template<class T>
class NumberSetting : public Setting {
protected:
    T initial;
    T value;
    T min;
    T max;
public:
    NumberSetting(
        T value, 
        T min=std::numeric_limits<T>::min(), 
        T max=std::numeric_limits<T>::max(),
        setting_format format=setting_format::simple
    ) : Setting(format), 
        initial(value), 
        value(value), 
        min(min), 
        max(max) 
    {}

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

    virtual void resetToDefault() override {
        value = initial;
    }

    virtual std::string toString() const override;

    static inline NumberSetting createPercent(T def) {
        return NumberSetting(def, 0.0, 1.0, setting_format::percent);
    }
};

#endif // DATA_SETTING_H_
