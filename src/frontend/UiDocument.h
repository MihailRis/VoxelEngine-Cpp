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

namespace scripting {
    class Environment;
}

struct uidocscript {
    int environment;
    bool onopen : 1;
    bool onclose : 1;
};

using uinodes_map = std::unordered_map<std::string, std::shared_ptr<gui::UINode>>;

class UiDocument {
    std::string id;
    uidocscript script;
    uinodes_map map;
    std::shared_ptr<gui::UINode> root;
    std::unique_ptr<scripting::Environment> env;
public:
    UiDocument(
        std::string id, 
        uidocscript script, 
        std::shared_ptr<gui::UINode> root, 
        std::unique_ptr<scripting::Environment> env
    );

    const std::string& getId() const;
    const uinodes_map& getMap() const;
    const std::shared_ptr<gui::UINode> getRoot() const;
    const uidocscript& getScript() const;
    int getEnvironment() const;
    /* Collect map of all uinodes having identifiers */
    static void collect(uinodes_map& map, std::shared_ptr<gui::UINode> node);

    static std::unique_ptr<UiDocument> read(int env, std::string namesp, fs::path file);
};

#endif // FRONTEND_UI_DOCUMENT_H_
