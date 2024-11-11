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

    size_t totalUpload = 0;
    size_t totalDownload = 0;
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
        if (res == CURLE_OK) {
            long size;
            if (!curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &size)) {
                totalUpload += size;
            }
            if (!curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &size)) {
                totalDownload += size;
            }
            totalDownload += buffer.size();
        }
        callback(res, std::move(buffer));
    }

    size_t getTotalUpload() const override {
        return totalUpload;
    }

    size_t getTotalDownload() const override {
        return totalDownload;
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

size_t Network::getTotalUpload() const {
    return http->getTotalUpload();
}

size_t Network::getTotalDownload() const {
    return http->getTotalDownload();
}

std::unique_ptr<Network> Network::create(const NetworkSettings& settings) {
    auto http = CurlHttp::create();
    return std::make_unique<Network>(std::move(http));
}
