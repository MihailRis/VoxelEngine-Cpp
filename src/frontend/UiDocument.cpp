#include "UiDocument.h"

#include "gui/UINode.h"
#include "gui/panels.h"
#include "InventoryView.h"
#include "../logic/scripting/scripting.h"
#include "../files/files.h"

UiDocument::UiDocument(
    std::string namesp, 
    uidocscript script, 
    std::shared_ptr<gui::UINode> root
) {
    collect(map, root);
}

void UiDocument::collect(uinodes_map& map, std::shared_ptr<gui::UINode> node) {
    const std::string& id = node->getId();
    if (!id.empty()) {
        map[id] = node;
    }
    auto container = dynamic_cast<gui::Container*>(node.get());
    if (container) {
        for (auto subnode : container->getNodes()) {
            collect(map, subnode);
        }
    }
}

std::unique_ptr<UiDocument> UiDocument::readInventory(
    std::string namesp,
    fs::path file,
    LevelFrontend* frontend,
    InventoryInteraction& interaction
) {
    const std::string text = files::read_string(file);
    auto env = scripting::create_environment();
    auto view = InventoryView::readXML(
        frontend, interaction, text, file.u8string(), *env
    );
    uidocscript script {};
    auto scriptFile = fs::path(file.u8string()+".lua");
    if (fs::is_regular_file(scriptFile)) {
        scripting::load_layout_script(scriptFile, script);
    }
    return std::make_unique<UiDocument>(namesp, script, view);
}
