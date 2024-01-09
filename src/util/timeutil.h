#ifndef UTIL_TIMEUTIL_H_
#define UTIL_TIMEUTIL_H_

#include "../typedefs.h"
#include <chrono>

namespace timeutil {
    class Timer {
        std::chrono::high_resolution_clock::time_point start;
    public:
        Timer();
        long stop();
    };

/*  Timer that stops and prints time when destructor called
 *  @example:
 *      {                                     // some scope (custom, function, if/else, cycle etc.)
 *      timeutil::ScopeLogTimer scopeclock();
 *      ...
 *      } */
    class ScopeLogTimer : public Timer{
            long long scopeid_;
        public:
            ScopeLogTimer(long long id);
            ~ScopeLogTimer();
    };

    float time_value(float hour, float minute, float second);
    void from_value(float value, int& hour, int& minute, int& second);
}

#endif // UTIL_TIMEUTIL_H_