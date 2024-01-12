#include "ContentPack.h"

using std::string;
using std::filesystem::path;

ContentPack::ContentPack(const string id,
                         const path folder) 
    : id(id), folder(folder) {
    
}
const string& ContentPack::getId() const {
    return id;
}

const path& ContentPack::getFolder() const {
    return folder;
}
