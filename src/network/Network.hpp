#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "typedefs.hpp"
#include "settings.hpp"
#include "util/Buffer.hpp"

namespace network {
    using OnResponse = std::function<void(std::vector<char>)>;
    using OnReject = std::function<void(const char*)>;

    class Http {
    public:
        virtual ~Http() {}

        virtual void get(
            const std::string& url,
            OnResponse onResponse,
            OnReject onReject=nullptr
        ) = 0;
        virtual size_t getTotalUpload() const = 0;
        virtual size_t getTotalDownload() const = 0;
    };

    class Network {
        std::unique_ptr<Http> http;
    public:
        Network(std::unique_ptr<Http> http);
        ~Network();

        void httpGet(
            const std::string& url,
            OnResponse onResponse,
            OnReject onReject = nullptr
        );

        size_t getTotalUpload() const;
        size_t getTotalDownload() const;

        static std::unique_ptr<Network> create(const NetworkSettings& settings);
    };
}
