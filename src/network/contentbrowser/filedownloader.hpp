#include <iostream>
#include <curl/curl.h>
#include <string>

class FileDownloader {
public:
    FileDownloader() {
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();
    }

    ~FileDownloader() {
        if (curl_handle) {
            curl_easy_cleanup(curl_handle);
        }
        curl_global_cleanup();
    }

    bool downloadFile(const std::string& url, const std::string& output_file) {
        if (!curl_handle) {
            std::cerr << "Failed to initialize CURL." << std::endl;
            return false;
        }

        FILE* file = fopen(output_file.c_str(), "wb");
        if (!file) {
            std::cerr << "Could not open file " << output_file << " for writing." << std::endl;
            return false;
        }

        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "User-Agent: MyUserAgent/1.0");
        headers = curl_slist_append(headers, "Accept: */*");
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl_handle);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            fclose(file);
            curl_slist_free_all(headers);
            return false;
        }

        long response_code;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            std::cerr << "HTTP response code: " << response_code << std::endl;
            fclose(file);
            curl_slist_free_all(headers);
            return false;
        }

        fclose(file);
        curl_slist_free_all(headers);
        return true;
    }

private:
    CURL* curl_handle;

    static size_t writeData(void* ptr, size_t size, size_t nmemb, FILE* stream) {
        return fwrite(ptr, size, nmemb, stream);
    }
};