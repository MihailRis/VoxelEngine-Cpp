#include "Network.hpp"

#include <curl/curl.h>
#include <stdexcept>

#include "debug/Logger.hpp"

using namespace network;

static debug::Logger logger("network");

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto& buffer = *reinterpret_cast<std::vector<char>*>(userdata);
    size_t psize = buffer.size();
    buffer.resize(psize + size * nmemb);
    std::memcpy(buffer.data() + psize, ptr, size * nmemb);
    return size * nmemb;
}

class CurlHttp : public Http {
    CURL* curl;
public:
    CurlHttp(CURL* curl) : curl(curl) {
    }

    virtual ~CurlHttp() {
        curl_easy_cleanup(curl);
    }

    void get(const std::string& url, const OnResponse& callback) override {
        std::vector<char> buffer;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        CURLcode res = curl_easy_perform(curl);
        callback(res, std::move(buffer));
    }

    static std::unique_ptr<CurlHttp> create() {
        if (auto curl = curl_easy_init()) {
            return std::make_unique<CurlHttp>(curl);
        }
        throw std::runtime_error("could not initialzie cURL");
    }
};


Network::Network(std::unique_ptr<Http> http) : http(std::move(http)) {
}

Network::~Network() = default;

void Network::httpGet(const std::string& url, const OnResponse& callback) {
    http->get(url, callback);
}

std::unique_ptr<Network> Network::create(const NetworkSettings& settings) {
    auto http = CurlHttp::create();
    return std::make_unique<Network>(std::move(http));
}
