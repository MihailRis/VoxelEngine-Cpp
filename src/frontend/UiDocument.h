#ifndef FRONTEND_UI_DOCUMENT_H_
#define FRONTEND_UI_DOCUMENT_H_

#include <string>
#include <memory>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

namespace gui {
    class UINode;
}

struct uidocscript {
    int environment;
    bool onopen : 1;
    bool onclose : 1;
};

using uinodes_map = std::unordered_map<std::string, std::shared_ptr<gui::UINode>>;

class UiDocument {
    std::string namesp;
    uidocscript script;
    uinodes_map map;
    std::shared_ptr<gui::UINode> root;
public:
    UiDocument(std::string namesp, uidocscript script, std::shared_ptr<gui::UINode> root);

    const uinodes_map& getMap() const;
    const std::string& getNamespace() const;
    const std::shared_ptr<gui::UINode> getRoot() const;

    /* Collect map of all uinodes having identifiers */
    static void collect(uinodes_map& map, std::shared_ptr<gui::UINode> node);

    static std::unique_ptr<UiDocument> read (std::string namesp, fs::path file);
};

#endif // FRONTEND_UI_DOCUMENT_H_
