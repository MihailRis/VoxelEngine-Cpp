#ifndef CONTENT_CONTENT_PACK_H_
#define CONTENT_CONTENT_PACK_H_

#include <string>
#include <filesystem>

class ContentPack {
    const std::string id;
    const std::filesystem::path folder;
public:
    ContentPack(const std::string id,
                const std::filesystem::path folder);

    const std::string& getId() const;
    const std::filesystem::path& getFolder() const;
};

#endif // CONTENT_CONTENT_PACK_H_
