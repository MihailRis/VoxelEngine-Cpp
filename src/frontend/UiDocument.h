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

class InventoryInteraction;
class LevelFrontend;

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

    /* Collect map of all uinodes having identifiers */
    static void collect(uinodes_map& map, std::shared_ptr<gui::UINode> node);

    /* @return root node is always an InventoryView */
    static std::unique_ptr<UiDocument> readInventory (
        std::string namesp,
        fs::path file,
        LevelFrontend* frontend,
        InventoryInteraction& interaction
    );
};

#endif // FRONTEND_UI_DOCUMENT_H_
