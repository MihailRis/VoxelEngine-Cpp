#ifndef DATA_SETTING_H_
#define DATA_SETTING_H_

#include <limits>
#include <string>

#include "../typedefs.h"

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

class NumberSetting : public Setting {
protected:
    number_t initial;
    number_t value;
    number_t min;
    number_t max;
public:
    NumberSetting(
        number_t value, 
        number_t min=std::numeric_limits<number_t>::min(), 
        number_t max=std::numeric_limits<number_t>::max(),
        setting_format format=setting_format::simple
    ) : Setting(format), 
        initial(value), 
        value(value), 
        min(min), 
        max(max)
    {}

    number_t& operator*() {
        return value;
    }

    number_t get() const {
        return value;
    }

    void set(number_t value) {
        this->value = value;
    }

    number_t getMin() const {
        return min;
    }

    number_t getMax() const {
        return max;
    }

    number_t getT() const {
        return (value - min) / (max - min);
    }

    virtual void resetToDefault() override {
        value = initial;
    }

    virtual std::string toString() const override;

    static inline NumberSetting createPercent(number_t def) {
        return NumberSetting(def, 0.0, 1.0, setting_format::percent);
    }
};

#endif // DATA_SETTING_H_
