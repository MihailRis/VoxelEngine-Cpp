#include "timeutil.h"

#include <iostream>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

timeutil::Timer::Timer() {
    start = high_resolution_clock::now();
}
long timeutil::Timer::stop() {
    return duration_cast<microseconds>(high_resolution_clock::now()-start).count();
}

timeutil::ScopeLogTimer::ScopeLogTimer(long long id) : scopeid_(id) {}

timeutil::ScopeLogTimer::~ScopeLogTimer() {
    std::cout << "Scope "<< scopeid_ <<" finished in "<< ScopeLogTimer::stop() << " micros. \n";
}

float timeutil::time_value(float hour, float minute, float second) {
    return (hour + (minute + second / 60.0f) / 60.0f) / 24.0f;
}

void timeutil::from_value(float value, int& hour, int& minute, int& second) {
    value *= 24;
    hour = value;
    value *= 60;
    minute = int(value) % 60;
    value *= 60;
    second = int(value) % 60;
}