#include <gtest/gtest.h>

#include <curl/curl.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    return size * nmemb;
}

TEST(curltest, curltest) {
    if (CURL* curl = curl_easy_init()) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, "https://github.com");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        res = curl_easy_perform(curl);
        std::cout << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
    }
}
