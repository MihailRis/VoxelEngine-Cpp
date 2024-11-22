#pragma once

#include <memory>
#include <vector>

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
            OnReject onReject=nullptr,
            long maxSize=0
        ) = 0;
        virtual size_t getTotalUpload() const = 0;
        virtual size_t getTotalDownload() const = 0;

        virtual void update() = 0;
    };

    class Socket {
    public:
        virtual int recv(char* buffer, size_t length) = 0;
        virtual int send(const char* buffer, size_t length) = 0;
        virtual void close() = 0;
        virtual bool isOpen() const = 0;

        virtual size_t getTotalUpload() const = 0;
        virtual size_t getTotalDownload() const = 0;
    };

    class Tcp {
    public:
        virtual ~Tcp() {}

        virtual std::shared_ptr<Socket> connect(
            const std::string& address, int port
        ) = 0;
    };

    class Network {
        std::unique_ptr<Http> http;
        std::unique_ptr<Tcp> tcp;
        std::vector<std::shared_ptr<Socket>> connections;
    public:
        Network(std::unique_ptr<Http> http, std::unique_ptr<Tcp> tcp);
        ~Network();

        void httpGet(
            const std::string& url,
            OnResponse onResponse,
            OnReject onReject = nullptr,
            long maxSize=0
        );

        std::shared_ptr<Socket> connect(const std::string& address, int port);

        size_t getTotalUpload() const;
        size_t getTotalDownload() const;

        void update();

        static std::unique_ptr<Network> create(const NetworkSettings& settings);
    };
}
