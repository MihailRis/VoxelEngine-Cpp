#ifndef DELEGATES_H_
#define DELEGATES_H_

#include <functional>
#include <string>

typedef std::function<void()> runnable;
typedef std::function<void(const std::string&)> stringconsumer;

#endif // DELEGATES_H_
