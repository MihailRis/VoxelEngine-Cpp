#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "typedefs.hpp"
#include "settings.hpp"
#include "util/Buffer.hpp"

namespace network {
    using OnResponse = std::function<void(int, std::vector<char>)>;

    class Http {
    public:
        virtual ~Http() {}

        virtual void get(const std::string& url, const OnResponse& callback) = 0;
    };

    class Network {
        std::unique_ptr<Http> http;
    public:
        Network(std::unique_ptr<Http> http);
        ~Network();

        void httpGet(const std::string& url, const OnResponse& callback);

        static std::unique_ptr<Network> create(const NetworkSettings& settings);
    };
}
