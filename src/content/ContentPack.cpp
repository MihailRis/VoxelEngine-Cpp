#include "ContentPack.h"

#include <stdexcept>

#include "../files/files.h"
#include "../coders/json.h"

namespace fs = std::filesystem;

const std::string ContentPack::PACKAGE_FILENAME = "package.json";
const std::string ContentPack::CONTENT_FILENAME = "content.json";

std::filesystem::path ContentPack::getContentFile() const {
    return folder/fs::path(CONTENT_FILENAME);
}

bool ContentPack::is_pack(std::filesystem::path folder) {
    return fs::is_regular_file(folder/fs::path(PACKAGE_FILENAME));
}

ContentPack ContentPack::read(std::filesystem::path folder) {
    auto root = files::read_json(folder/fs::path(PACKAGE_FILENAME));
    ContentPack pack;
    root->str("id", pack.id);
    root->str("title", pack.title);
    root->str("version", pack.version);
    pack.folder = folder;
    if (pack.id == "none")
        throw std::runtime_error("content-pack id is none: "+folder.u8string());
    return pack;
}

void ContentPack::scan(fs::path rootfolder,
                       std::vector<ContentPack>& packs) {
    if (!fs::is_directory(rootfolder)) {
        return;
    }
    for (auto entry : fs::directory_iterator(rootfolder)) {
        fs::path folder = entry.path();
        if (!fs::is_directory(folder))
            continue;
        if (!is_pack(folder))
            continue;
        packs.push_back(read(folder));
    }
}
