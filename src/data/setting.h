#ifndef DATA_SETTING_H_
#define DATA_SETTING_H_

#include <limits>
#include <string>
#include <vector>
#include <unordered_map>

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

using observer_handler = std::shared_ptr<int>;

template<class T>
class Observers {
    int nextid = 1;
    std::unordered_map<int, consumer<T>> observers;
public:
    observer_handler observe(consumer<T> callback) {
        const int id = nextid++;
        observers.emplace(id, callback);
        return std::shared_ptr<int>(new int(id), [this](int* id) {
            observers.erase(*id);
            delete id;
        });
    }

    void notify(T value) {
        for (auto& entry : observers) {
            entry.second(value);
        }
    }
};

class NumberSetting : public Setting {
protected:
    number_t initial;
    number_t value;
    number_t min;
    number_t max;
    Observers<number_t> observers;
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
        observers.notify(value);
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

    observer_handler observe(consumer<number_t> callback) {
        return observers.observe(callback);
    }

    virtual void resetToDefault() override {
        set(initial);
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
    Observers<integer_t> observers;
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
        observers.notify(value);
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

    observer_handler observe(consumer<integer_t> callback) {
        return observers.observe(callback);
    }

    virtual void resetToDefault() override {
        set(initial);
    }

    virtual std::string toString() const override;
};

class FlagSetting : public Setting {
protected:
    bool initial;
    bool value;
    Observers<bool> observers;
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
        observers.notify(value);
    }

    observer_handler observe(consumer<bool> callback) {
        return observers.observe(callback);
    }

    virtual void resetToDefault() override {
        set(initial);
    }

    virtual std::string toString() const override;
};

#endif // DATA_SETTING_H_
