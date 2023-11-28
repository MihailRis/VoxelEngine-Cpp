#ifndef UTIL_TIMEUTIL_H_
#define UTIL_TIMEUTIL_H_

namespace timeutil {
    float time_value(float hour, float minute, float second);
    void from_value(float value, int& hour, int& minute, int& second);
}

#endif // UTIL_TIMEUTIL_H_