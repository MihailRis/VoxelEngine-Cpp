#include <gtest/gtest.h>

#include "network/Network.hpp"
#include "coders/json.hpp"

TEST(curltest, curltest) {
    NetworkSettings settings {};
    auto network = network::Network::create(settings);
    network->httpGet(
        "https://raw.githubusercontent.com/MihailRis/VoxelEngine-Cpp/refs/"
        "heads/curl/res/content/base/blocks/lamp.json",
        [=](int code, std::vector<char> data) {
            auto v = std::string_view(data.data(), data.size());
            auto value = json::parse(v);
            std::cout << value << std::endl;
        }
    );
    std::cout << "upload: " << network->getTotalUpload() << " B" << std::endl;
    std::cout << "download: " << network->getTotalDownload() << " B" << std::endl;
}
