#include "setting.h"

#include "../util/stringutil.h"

template<class T>
std::string NumberSetting<T>::toString() const {
    switch (getFormat()) {
        case setting_format::simple:
            return util::to_string(value);
        case setting_format::percent:
            return std::to_string(static_cast<int64_t>(value * 100)) + "%";
        default:
            return "invalid format";
    }
}

template class NumberSetting<float>;
template class NumberSetting<double>;
template class NumberSetting<int>;
template class NumberSetting<uint>;
