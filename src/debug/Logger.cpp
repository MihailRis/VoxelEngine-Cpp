#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <utility>

using namespace debug;

std::ofstream Logger::file;
std::mutex Logger::mutex;
std::string Logger::utcOffset = "";
unsigned Logger::moduleLen = 20;

LogMessage::~LogMessage() {
    logger->log(level, ss.str());
}

Logger::Logger(std::string name) : name(std::move(name)) {
}

void Logger::log(
    LogLevel level, const std::string& name, const std::string& message
) {
    using namespace std::chrono;

    std::stringstream ss;
    switch (level) {
        case LogLevel::debug:
#ifdef NDEBUG
            return;
#endif
            ss << "[D]";
            break;
        case LogLevel::info:
            ss << "[I]";
            break;
        case LogLevel::warning:
            ss << "[W]";
            break;
        case LogLevel::error:
            ss << "[E]";
            break;
    }
    time_t tm = std::time(nullptr);
    auto ms =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()) %
        1000;
    ss << " " << std::put_time(std::localtime(&tm), "%Y/%m/%d %T");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    ss << utcOffset << " [" << std::setfill(' ') << std::setw(moduleLen) << name
       << "] ";
    ss << message;
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto string = ss.str();
        if (file.good()) {
            file << string << '\n';
            file.flush();
        }
        std::cout << string << std::endl;
    }
}

void Logger::init(const std::string& filename) {
    file.open(filename);

    time_t tm = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&tm), "%z");
    utcOffset = ss.str();
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex);
    file.flush();
}

void Logger::log(LogLevel level, std::string message) {
    log(level, name, std::move(message));
}
