#include "setting.hpp"

#include "util/stringutil.hpp"

std::string NumberSetting::toString() const {
    switch (getFormat()) {
        case setting_format::simple:
            return util::to_string(value);
        case setting_format::percent:
            return std::to_string(static_cast<integer_t>(round(value * 100))) +
                   "%";
        default:
            return "invalid format";
    }
}

std::string IntegerSetting::toString() const {
    switch (getFormat()) {
        case setting_format::simple:
            return util::to_string(value);
        case setting_format::percent:
            return std::to_string(value) + "%";
        default:
            return "invalid format";
    }
}

std::string FlagSetting::toString() const {
    switch (getFormat()) {
        case setting_format::simple:
            return value ? "true" : "false";
        default:
            return "invalid format";
    }
}

std::string StringSetting::toString() const {
    return value;
}
