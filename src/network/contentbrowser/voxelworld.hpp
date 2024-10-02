#pragma once
#include <iostream>
#include <string>
#include "json/json.h"
#include <curl/curl.h>
#include <filesystem>
#include <archive.h>
#include "filedownloader.hpp"
#include "zipextractor.hpp"
#include <archive_entry.h>

namespace fs = std::filesystem;
size_t WriteStringCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t WriteFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* out = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    out->write(static_cast<const char*>(contents), totalSize);
    return totalSize;
}


void download_file(const std::string& url, const std::string& output_path) {
    CURL* curl;
    CURLcode res;
    std::ofstream out_file(output_path, std::ios::binary);

    if (!out_file) {
        std::cerr << "Could not open file for writing: " << output_path << std::endl;
        return;
    }

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_file);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    out_file.close();
}
void checkAndAddBetaValue(const std::string& jsonFilePath,const std::string& value) {
    std::ifstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonFilePath << std::endl;
        return;
    }

    Json::Value jsonData;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;

    if (!Json::parseFromStream(readerBuilder, jsonFile, &jsonData, &errs)) {
        std::cerr << "Error parsing JSON: " << errs << std::endl;
        return;
    }
    jsonFile.close();

    bool found = false;
    for (const auto& texture : jsonData["textures"]) {
        if (texture.asString() == value) {
            found = true;
            break;
        }
    }

    if (!found) {
        jsonData["textures"].append(value);
        std::cout << value << " added to textures." << std::endl;

        std::ofstream outFile(jsonFilePath);
        if (outFile.is_open()) {
            outFile << jsonData;
            outFile.close();
            std::cout << "Updated JSON file saved: " << jsonFilePath << std::endl;
        } else {
            std::cerr << "Failed to open file for writing: " << jsonFilePath << std::endl;
        }
    } else {
        std::cout << value <<" already exists in textures." << std::endl;
    }
}

bool stringToJsonValue(const std::string& jsonString, Json::Value& jsonValue) {
    Json::CharReaderBuilder readerBuilder;
    std::string errs;   
    std::istringstream jsonStream(jsonString);
    bool get = Json::parseFromStream(readerBuilder, jsonStream, &jsonValue, &errs);
    if (!get) {
        std::cerr << "Error parsing JSON: " << errs << std::endl;
    }
    return get;
}
size_t writeData(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
bool downloadIcon(const std::string& url, const std::string& savePath) {
    CURL* curl;
    CURLcode res;
    FILE* file;

    curl = curl_easy_init();
    if(curl) {
        file = fopen(savePath.c_str(), "wb");
        if (!file) {
            std::cerr << "Could not open file for writing: " << savePath << std::endl;
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            std::cerr << "HTTP request failed with response code: " << response_code << std::endl;
            fclose(file);
            return false;
        }

        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            fclose(file);
            return false;
        }

        fclose(file);
        curl_easy_cleanup(curl);
        return true;
    }
    return false;
}


std::string sendGetRequest(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteStringCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            readBuffer.clear();
        }

        curl_slist_free_all(headers); 
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}
bool check_path(std::string packid) {
    bool check;
    if (!fs::exists("res/textures/gui/cache/"+packid)) {
        fs::create_directories("res/textures/gui/cache/"+packid);
        check = true;
    } else {
        check = false;
    }
    return check;
}

class VoxelWorldAPI {
public:
    int page;
    Json::Value get_mods() {
        std::string response = sendGetRequest(get_mods_url + "?item_count=10&page="+std::to_string(page));
        Json::Value mods;
        bool check = stringToJsonValue(response, mods);
        mods["page"] = page;
        if (!check)
            return mods;
        if (check)
            this->mods = mods;
        return mods;
    }
    void download_pack(std::string mod_title) {
        Json::Value mod_data = get_mod(mod_title);
        Json::Value page_id;
        std::string mod_id,version;
        mod_id = mod_data["data"]["id"].asString();
        version = mod_data["data"]["detail_description"]["version"].asString();
        std::string response = sendGetRequest(base_url+"/api/v1/versions/"+mod_id+"?type=mod&page=1&item_count=1");
        bool check = stringToJsonValue(response, page_id);
        if (!check)
            return;
        
        
        response = sendGetRequest(base_url+"/api/v1/versions/"+mod_id+"?type=mod&page="+page_id["meta"]["total"].asString()+"&item_count=1");
        page_id = Json::nullValue;
        check = stringToJsonValue(response, page_id);
        if (!check)
            return;
        std::string link = base_url + "/mods/"+mod_id+"/version/"+page_id["data"][0]["id"].asString()+"/download";
        if (!fs::exists("content")) 
            fs::create_directory("content");
        std::string save_path = "content/"+mod_id+"_"+page_id["data"][0]["id"].asString()+".zip";
        
        fd.downloadFile(link,save_path);
        ArchiveExtractor ae(save_path);
        ae.extract();   
    }

    Json::Value get_mod(std::string mod_title) {
        int id = -1; 
        if (mods["page"].asInt() != page)
            mods = get_mods();
        Json::Value _mods = mods["data"];
        
        for (int i = 0; i < _mods.size(); i++) {
            if (_mods[i]["title"].asString() == mod_title) { 
                id = _mods[i]["id"].asInt(); 
                if (!fs::exists( "res/textures/gui/cache/" + _mods[i]["id"].asString() + "/icon.png")) {
                    check_path(_mods[i]["id"].asString());
                    fd.downloadFile(base_url + _mods[i]["pathLogo"].asString(), "res/textures/gui/cache/" + _mods[i]["id"].asString() + "/icon.png");
                }
                break; 
            }
        }

        if (id == -1) {
            std::cerr << "Мод с заголовком '" << mod_title << "' не найден." << std::endl;
            return Json::Value(); 
        }

        std::string response = sendGetRequest(get_mods_url + "/" + std::to_string(id));
        Json::Value mod;
        bool check = stringToJsonValue(response, mod);
        if (!check)
            return mod;

        return mod;
    }

    Json::Value get_mod_from_mods(std::string mod_title) {
        if (mods["page"].asInt() != page) {
            mods = get_mods();
        }
        
        Json::Value _mods = mods["data"];
        for (int i = 0; i < _mods.size(); i++) {
            if (_mods[i]["title"].asString() == mod_title) {
                std::cout << "Загружается иконка для мода: " << mod_title << " с ID: " << _mods[i]["id"].asString() << std::endl;
                if (!fs::exists("res/textures/gui/cache/" + _mods[i]["id"].asString() + "/icon.png")) {
                    check_path(_mods[i]["id"].asString());
                    fd.downloadFile(base_url + _mods[i]["pathLogo"].asString(), "res/textures/gui/cache/" + _mods[i]["id"].asString() + "/icon.png");
                }
                return _mods[i];
            }
        }
        std::cerr << "Мод с заголовком '" << mod_title << "' не найден в модах." << std::endl;
        return Json::Value();
    }


    

private:
    
    std::string base_url = "https://voxelworld.ru";
    std::string get_mods_url = base_url+"/api/v1/mods";
    Json::Value mods = get_mods();
    FileDownloader fd;
};
