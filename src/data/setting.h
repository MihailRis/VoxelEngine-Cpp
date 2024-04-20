#ifndef DATA_SETTING_H_
#define DATA_SETTING_H_

#include <limits>
#include <string>
#include <vector>

#include "../typedefs.h"
#include "../delegates.h"

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
    std::vector<consumer<number_t>> consumers;
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
        if (value == this->value) {
            return;
        }
        this->value = value;
        for (auto& callback : consumers) {
            callback(value);
        }
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

    void observe(consumer<number_t> callback) {
        consumers.push_back(callback);
    }

    virtual void resetToDefault() override {
        value = initial;
    }

    virtual std::string toString() const override;

    static inline NumberSetting createPercent(number_t def) {
        return NumberSetting(def, 0.0, 1.0, setting_format::percent);
    }
};

class IntegerSetting : public Setting {
protected:
    integer_t initial;
    integer_t value;
    integer_t min;
    integer_t max;
    std::vector<consumer<integer_t>> consumers;
public:
    IntegerSetting(
        integer_t value, 
        integer_t min=std::numeric_limits<integer_t>::min(), 
        integer_t max=std::numeric_limits<integer_t>::max(),
        setting_format format=setting_format::simple
    ) : Setting(format), 
        initial(value), 
        value(value), 
        min(min), 
        max(max)
    {}

    integer_t& operator*() {
        return value;
    }

    integer_t get() const {
        return value;
    }

    void set(integer_t value) {
        if (value == this->value) {
            return;
        }
        this->value = value;
        for (auto& callback : consumers) {
            callback(value);
        }
    }

    integer_t getMin() const {
        return min;
    }

    integer_t getMax() const {
        return max;
    }

    integer_t getT() const {
        return (value - min) / (max - min);
    }

    void observe(consumer<integer_t> callback) {
        consumers.push_back(callback);
    }

    virtual void resetToDefault() override {
        value = initial;
    }

    virtual std::string toString() const override;
};

class FlagSetting : public Setting {
protected:
    bool initial;
    bool value;
    std::vector<consumer<bool>> consumers;
public:
    FlagSetting(
        bool value,
        setting_format format=setting_format::simple
    ) : Setting(format),
        initial(value),
        value(value)
    {}

    bool& operator*() {
        return value;
    }

    bool get() const {
        return value;
    }

    void set(bool value) {
        if (value == this->value) {
            return;
        }
        this->value = value;
        for (auto& callback : consumers) {
            callback(value);
        }
    }

    void observe(consumer<bool> callback) {
        consumers.push_back(callback);
    }

    virtual void resetToDefault() override {
        value = initial;
    }

    virtual std::string toString() const override;
};

#endif // DATA_SETTING_H_
