#include "setting.h"

#include "../util/stringutil.h"

std::string NumberSetting::toString() const {
    switch (getFormat()) {
        case setting_format::simple:
            return util::to_string(value);
        case setting_format::percent:
            return std::to_string(static_cast<int>(value * 100)) + "%";
        default:
            return "invalid format";
    }
}
