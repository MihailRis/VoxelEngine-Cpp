#ifndef CONTENT_CONTENT_PACK_H_
#define CONTENT_CONTENT_PACK_H_

#include <string>
#include <filesystem>

struct ContentPack {
    std::string id;
    std::filesystem::path folder;
};

#endif // CONTENT_CONTENT_PACK_H_
