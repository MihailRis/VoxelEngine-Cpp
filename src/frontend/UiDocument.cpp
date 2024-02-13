#include "UiDocument.h"

#include <iostream>
#include "gui/UINode.h"
#include "gui/panels.h"
#include "InventoryView.h"
#include "../logic/scripting/scripting.h"
#include "../files/files.h"
#include "../frontend/gui/gui_xml.h"

UiDocument::UiDocument(
    std::string id, 
    uidocscript script, 
    std::shared_ptr<gui::UINode> root,
    std::unique_ptr<scripting::Environment> env
) : id(id), script(script), root(root), env(std::move(env)) {
    collect(map, root);
}


const uinodes_map& UiDocument::getMap() const {
    return map;
}

const std::string& UiDocument::getId() const {
    return id;
}

const std::shared_ptr<gui::UINode> UiDocument::getRoot() const {
    return root;
}

const std::shared_ptr<gui::UINode> UiDocument::get(const std::string& id) const {
    auto found = map.find(id);
    if (found == map.end()) {
        return nullptr;
    }
    return found->second;
}

const uidocscript& UiDocument::getScript() const {
    return script;
}

int UiDocument::getEnvironment() const {
    return env->getId();
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

std::unique_ptr<UiDocument> UiDocument::read(AssetsLoader& loader, int penv, std::string namesp, fs::path file) {
    const std::string text = files::read_string(file);
    auto xmldoc = xml::parse(file.u8string(), text);

    auto env = scripting::create_doc_environment(penv, namesp);
    gui::UiXmlReader reader(*env, loader);
    InventoryView::createReaders(reader);
    auto view = reader.readXML(
        file.u8string(), xmldoc->getRoot()
    );
    uidocscript script {};
    auto scriptFile = fs::path(file.u8string()+".lua");
    if (fs::is_regular_file(scriptFile)) {
        scripting::load_layout_script(env->getId(), namesp, scriptFile, script);
    }
    return std::make_unique<UiDocument>(namesp, script, view, std::move(env));
}
