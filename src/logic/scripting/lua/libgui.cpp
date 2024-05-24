#include "lua_commons.hpp"
#include "api_lua.hpp"
#include "lua_util.hpp"
#include "LuaState.hpp"

#include "../scripting.hpp"
#include "../../../engine.hpp"
#include "../../../assets/Assets.hpp"
#include "../../../items/Inventories.hpp"
#include "../../../graphics/ui/gui_util.hpp"
#include "../../../graphics/ui/elements/UINode.hpp"
#include "../../../graphics/ui/elements/Button.hpp"
#include "../../../graphics/ui/elements/Image.hpp"
#include "../../../graphics/ui/elements/CheckBox.hpp"
#include "../../../graphics/ui/elements/TextBox.hpp"
#include "../../../graphics/ui/elements/TrackBar.hpp"
#include "../../../graphics/ui/elements/Panel.hpp"
#include "../../../graphics/ui/elements/Menu.hpp"
#include "../../../graphics/ui/elements/InventoryView.hpp"
#include "../../../frontend/UiDocument.hpp"
#include "../../../frontend/locale.hpp"
#include "../../../util/stringutil.hpp"
#include "../../../world/Level.hpp"

using namespace gui;

namespace scripting {
    extern lua::LuaState* state;
}

struct DocumentNode {
    UiDocument* document;
    std::shared_ptr<UINode> node;
};

using namespace scripting;

static DocumentNode getDocumentNode(lua_State*, const std::string& name, const std::string& nodeName) {
    auto doc = engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        throw std::runtime_error("document '"+name+"' not found");
    }
    auto node = doc->get(nodeName);
    if (node == nullptr) {
        throw std::runtime_error("document '"+name+"' has no element with id '"+nodeName+"'");
    }
    return {doc, node};
}

static DocumentNode getDocumentNode(lua_State* L, int idx=1) {
    lua_getfield(L, idx, "docname");
    lua_getfield(L, idx, "name");
    auto docname = lua_tostring(L, -2);
    auto name = lua_tostring(L, -1);
    auto node = getDocumentNode(L, docname, name);
    lua_pop(L, 2);
    return node;
}

static int l_menu_back(lua_State* L) {
    auto node = getDocumentNode(L);
    auto menu = dynamic_cast<Menu*>(node.node.get());
    menu->back();
    return 0;
}

static int l_menu_reset(lua_State* L) {
    auto node = getDocumentNode(L);
    auto menu = dynamic_cast<Menu*>(node.node.get());
    menu->reset();
    return 0;
}

static int l_textbox_paste(lua_State* L) {
    auto node = getDocumentNode(L);
    auto box = dynamic_cast<TextBox*>(node.node.get());
    auto text = lua_tostring(L, 2);
    box->paste(util::str2wstr_utf8(text));
    return 0;
}

static int l_container_add(lua_State* L) {
    auto docnode = getDocumentNode(L);
    auto node = dynamic_cast<Container*>(docnode.node.get());
    auto xmlsrc = lua_tostring(L, 2);
    try {
        auto subnode = guiutil::create(xmlsrc, docnode.document->getEnvironment());
        node->add(subnode);
        UINode::getIndices(subnode, docnode.document->getMapWriteable());
    } catch (const std::exception& err) {
        throw std::runtime_error(err.what());
    }
    return 0;
}

static int l_container_clear(lua_State* L) {
    auto node = getDocumentNode(L, 1);
    if (auto container = std::dynamic_pointer_cast<Container>(node.node)) {
        container->clear();
    }
    return 0;
}

static int l_move_into(lua_State* L) {
    auto node = getDocumentNode(L, 1);
    auto dest = getDocumentNode(L, 2);
    UINode::moveInto(node.node, std::dynamic_pointer_cast<Container>(dest.node));
    return 0;
}

static int p_get_inventory(UINode* node) {
    if (auto inventory = dynamic_cast<InventoryView*>(node)) {
        auto inv = inventory->getInventory();
        return state->pushinteger(inv ? inv->getId() : 0);
    }
    return 0;
}

static int p_get_reset(UINode* node) {
    if (dynamic_cast<Menu*>(node)) {
        return state->pushcfunction(l_menu_reset);
    }
    return 0;
}

static int p_get_back(UINode* node) {
    if (dynamic_cast<Menu*>(node)) {
        return state->pushcfunction(l_menu_back);
    }
    return 0;
}

static int p_get_paste(UINode* node) {
    if (dynamic_cast<TextBox*>(node)) {
        return state->pushcfunction(l_textbox_paste);
    }
    return 0;
}

static int p_get_page(UINode* node) {
    if (auto menu = dynamic_cast<Menu*>(node)) {
        return state->pushstring(menu->getCurrent().name);
    }
    return 0;
}

static int p_is_checked(UINode* node) {
    if (auto box = dynamic_cast<CheckBox*>(node)) {
        return state->pushboolean(box->isChecked());
    } else if (auto box = dynamic_cast<FullCheckBox*>(node)) {
        return state->pushboolean(box->isChecked());
    }
    return 0;
}

static int p_get_value(UINode* node) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        return state->pushnumber(bar->getValue());
    }
    return 0;
}

static int p_get_min(UINode* node) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        return state->pushnumber(bar->getMin());
    }
    return 0;
}

static int p_get_max(UINode* node) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        return state->pushnumber(bar->getMax());
    }
    return 0;
}

static int p_get_step(UINode* node) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        return state->pushnumber(bar->getStep());
    }
    return 0;
}

static int p_get_track_width(UINode* node) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        return state->pushnumber(bar->getTrackWidth());
    }
    return 0;
}

static int p_get_track_color(UINode* node) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        return lua::pushcolor_arr(state->getLua(), bar->getTrackColor());
    }
    return 0;
}

static int p_is_valid(UINode* node) {
    if (auto box = dynamic_cast<TextBox*>(node)) {
        return state->pushboolean(box->validate());
    }
    return 0;
}

static int p_get_caret(UINode* node) {
    if (auto box = dynamic_cast<TextBox*>(node)) {
        return state->pushinteger(static_cast<integer_t>(box->getCaret()));
    }
    return 0;
}

static int p_get_placeholder(UINode* node) {
    if (auto box = dynamic_cast<TextBox*>(node)) {
        return state->pushstring(util::wstr2str_utf8(box->getPlaceholder()));
    }
    return 0;
}

static int p_get_text(UINode* node) {
    if (auto button = dynamic_cast<Button*>(node)) {
        return state->pushstring(util::wstr2str_utf8(button->getText()));
    } else if (auto label = dynamic_cast<Label*>(node)) {
        return state->pushstring(util::wstr2str_utf8(label->getText()));
    } else if (auto box = dynamic_cast<TextBox*>(node)) {
        return state->pushstring(util::wstr2str_utf8(box->getText()));
    }
    return 0;
}

static int p_get_editable(UINode* node) {
    if (auto box = dynamic_cast<TextBox*>(node)) {
        return state->pushboolean(box->isEditable());
    }
    return 0;
}

static int p_get_src(UINode* node) {
    if (auto image = dynamic_cast<Image*>(node)) {
        return state->pushstring(image->getTexture());
    }
    return 0;
}

static int p_get_add(UINode* node) {
    if (dynamic_cast<Container*>(node)) {
        return state->pushcfunction(l_container_add);
    }
    return 0;
}

static int p_get_clear(UINode* node) {
    if (dynamic_cast<Container*>(node)) {
        return state->pushcfunction(l_container_clear);
    }
    return 0;
}

static int p_get_color(UINode* node) {
    return lua::pushcolor_arr(state->getLua(), node->getColor());
}
static int p_get_hover_color(UINode* node) {
    return lua::pushcolor_arr(state->getLua(), node->getHoverColor());
}
static int p_get_pressed_color(UINode* node) {
    return lua::pushcolor_arr(state->getLua(), node->getPressedColor());
}
static int p_get_pos(UINode* node) {
    return lua::pushvec2_arr(state->getLua(), node->getPos());
}
static int p_get_wpos(UINode* node) {
    return lua::pushvec2_arr(state->getLua(), node->calcPos());
}
static int p_get_size(UINode* node) {
    return lua::pushvec2_arr(state->getLua(), node->getSize());
}
static int p_is_interactive(UINode* node) {
    return state->pushboolean(node->isInteractive());
}
static int p_is_visible(UINode* node) {
    return state->pushboolean(node->isVisible());
}
static int p_is_enabled(UINode* node) {
    return state->pushboolean(node->isEnabled());
}
static int p_move_into(UINode*) {
    return state->pushcfunction(l_move_into);
}
static int p_get_focused(UINode* node) {
    return state->pushboolean(node->isFocused());
}

static int l_gui_getattr(lua_State* L) {
    auto docname = lua_tostring(L, 1);
    auto element = lua_tostring(L, 2);
    auto attr = lua_tostring(L, 3);
    auto docnode = getDocumentNode(L, docname, element);
    auto node = docnode.node;

    static const std::unordered_map<std::string_view, std::function<int(UINode*)>> getters {
        {"color", p_get_color},
        {"hoverColor", p_get_hover_color},
        {"pressedColor", p_get_pressed_color},
        {"pos", p_get_pos},
        {"wpos", p_get_wpos},
        {"size", p_get_size},
        {"interactive", p_is_interactive},
        {"visible", p_is_visible},
        {"enabled", p_is_enabled},
        {"move_into", p_move_into},
        {"add", p_get_add},
        {"clear", p_get_clear},
        {"placeholder", p_get_placeholder},
        {"valid", p_is_valid},
        {"caret", p_get_caret},
        {"text", p_get_text},
        {"editable", p_get_editable},
        {"src", p_get_src},
        {"value", p_get_value},
        {"min", p_get_min},
        {"max", p_get_max},
        {"step", p_get_step},
        {"trackWidth", p_get_track_width},
        {"trackColor", p_get_track_color},
        {"checked", p_is_checked},
        {"page", p_get_page},
        {"back", p_get_back},
        {"reset", p_get_reset},
        {"paste", p_get_paste},
        {"inventory", p_get_inventory},
        {"focused", p_get_focused},
    };
    auto func = getters.find(attr);
    if (func != getters.end()) {
        return func->second(node.get());
    }
    return 0;
}

static void p_set_color(UINode* node, int idx) {
    node->setColor(state->tocolor(idx));
}
static void p_set_hover_color(UINode* node, int idx) {
    node->setHoverColor(state->tocolor(idx));
}
static void p_set_pressed_color(UINode* node, int idx) {
    node->setPressedColor(state->tocolor(idx));
}
static void p_set_pos(UINode* node, int idx) {
    node->setPos(state->tovec2(idx));
}
static void p_set_wpos(UINode* node, int idx) {
    node->setPos(state->tovec2(idx)-node->calcPos());
}
static void p_set_size(UINode* node, int idx) {
    node->setSize(state->tovec2(idx));
}
static void p_set_interactive(UINode* node, int idx) {
    node->setInteractive(state->toboolean(idx));
}
static void p_set_visible(UINode* node, int idx) {
    node->setVisible(state->toboolean(idx));
}
static void p_set_enabled(UINode* node, int idx) {
    node->setEnabled(state->toboolean(idx));
}
static void p_set_placeholder(UINode* node, int idx) {
    if (auto box = dynamic_cast<TextBox*>(node)) {
        box->setPlaceholder(util::str2wstr_utf8(state->tostring(idx)));
    }
}
static void p_set_text(UINode* node, int idx) {
    if (auto label = dynamic_cast<Label*>(node)) {
        label->setText(util::str2wstr_utf8(state->tostring(idx)));
    } else if (auto button = dynamic_cast<Button*>(node)) {
        button->setText(util::str2wstr_utf8(state->tostring(idx)));
    } else if (auto box = dynamic_cast<TextBox*>(node)) {
        box->setText(util::str2wstr_utf8(state->tostring(idx)));
    }
}
static void p_set_caret(UINode* node, int idx) {
    if (auto box = dynamic_cast<TextBox*>(node)) {
        box->setCaret(static_cast<ptrdiff_t>(state->tointeger(idx)));
    }
}
static void p_set_editable(UINode* node, int idx) {
    if (auto box = dynamic_cast<TextBox*>(node)) {
        box->setEditable(state->toboolean(idx));
    }
}
static void p_set_src(UINode* node, int idx) {
    if (auto image = dynamic_cast<Image*>(node)) {
        image->setTexture(state->requireString(idx));
    }
}
static void p_set_value(UINode* node, int idx) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        bar->setValue(state->tonumber(idx));
    }
}
static void p_set_min(UINode* node, int idx) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        bar->setMin(state->tonumber(idx));
    }
}
static void p_set_max(UINode* node, int idx) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        bar->setMax(state->tonumber(idx));
    }
}
static void p_set_step(UINode* node, int idx) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        bar->setStep(state->tonumber(idx));
    }
}
static void p_set_track_width(UINode* node, int idx) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        bar->setTrackWidth(state->tointeger(idx));
    }
}
static void p_set_track_color(UINode* node, int idx) {
    if (auto bar = dynamic_cast<TrackBar*>(node)) {
        bar->setTrackColor(state->tocolor(idx));
    }
}
static void p_set_checked(UINode* node, int idx) {
    if (auto box = dynamic_cast<CheckBox*>(node)) {
        box->setChecked(state->toboolean(idx));
    } else if (auto box = dynamic_cast<FullCheckBox*>(node)) {
        box->setChecked(state->toboolean(idx));
    }
}
static void p_set_page(UINode* node, int idx) {
    if (auto menu = dynamic_cast<Menu*>(node)) {
        menu->setPage(state->tostring(idx));
    }
}
static void p_set_inventory(UINode* node, int idx) {
    if (auto view = dynamic_cast<InventoryView*>(node)) {
        auto inventory = level->inventories->get(state->tointeger(idx));
        if (inventory == nullptr) {
            view->unbind();
        } else {
            view->bind(inventory, content);
        }
    }
}
static void p_set_focused(std::shared_ptr<UINode> node, int idx) {
    if (state->toboolean(idx) && !node->isFocused()) {
        scripting::engine->getGUI()->setFocus(node);
    } else if (node->isFocused()){
        node->defocus();
    }
}

static int l_gui_setattr(lua_State* L) {
    auto docname = lua_tostring(L, 1);
    auto element = lua_tostring(L, 2);
    auto attr = lua_tostring(L, 3);

    auto docnode = getDocumentNode(L, docname, element);
    auto node = docnode.node;

    static const std::unordered_map<std::string_view, std::function<void(UINode*,int)>> setters {
        {"color", p_set_color},
        {"hoverColor", p_set_hover_color},
        {"pressedColor", p_set_pressed_color},
        {"pos", p_set_pos},
        {"wpos", p_set_wpos},
        {"size", p_set_size},
        {"interactive", p_set_interactive},
        {"visible", p_set_visible},
        {"enabled", p_set_enabled},
        {"placeholder", p_set_placeholder},
        {"text", p_set_text},
        {"editable", p_set_editable},
        {"src", p_set_src},
        {"caret", p_set_caret},
        {"value", p_set_value},
        {"min", p_set_min},
        {"max", p_set_max},
        {"step", p_set_step},
        {"trackWidth", p_set_track_width},
        {"trackColor", p_set_track_color},
        {"checked", p_set_checked},
        {"page", p_set_page},
        {"inventory", p_set_inventory},
    };
    auto func = setters.find(attr);
    if (func != setters.end()) {
        func->second(node.get(), 4);
    }
    static const std::unordered_map<std::string_view, std::function<void(std::shared_ptr<UINode>,int)>> setters2 {
        {"focused", p_set_focused},
    };
    auto func2 = setters2.find(attr);
    if (func2 != setters2.end()) {
        func2->second(node, 4);
    }
    return 0;
}

static int l_gui_get_env(lua_State* L) {
    auto name = lua_tostring(L, 1);
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        throw std::runtime_error("document '"+std::string(name)+"' not found");
    }
    lua_getglobal(L, lua::LuaState::envName(*doc->getEnvironment()).c_str());
    return 1;
}

static int l_gui_str(lua_State* L) {
    auto text = util::str2wstr_utf8(lua_tostring(L, 1));
    if (!lua_isnoneornil(L, 2)) {
        auto context = util::str2wstr_utf8(lua_tostring(L, 2));
        lua_pushstring(L, util::wstr2str_utf8(langs::get(text, context)).c_str());
    } else {
        lua_pushstring(L, util::wstr2str_utf8(langs::get(text)).c_str());
    }
    return 1;
}

static int l_gui_reindex(lua_State* L) {
    auto name = lua_tostring(L, 1);
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        throw std::runtime_error("document '"+std::string(name)+"' not found");
    }
    doc->rebuildIndices();
    return 0;
}

/// @brief gui.get_locales_info() -> table of tables 
static int l_gui_get_locales_info(lua_State* L) {
    auto& locales = langs::locales_info;
    lua_createtable(L, 0, locales.size());
    for (auto& entry : locales) {
        lua_createtable(L, 0, 1);
        lua_pushstring(L, entry.second.name.c_str());
        lua_setfield(L, -2, "name");
        lua_setfield(L, -2, entry.first.c_str());
    }
    return 1;
}

static int l_gui_getviewport(lua_State* L) {
    return lua::pushvec2_arr(L, scripting::engine->getGUI()->getContainer()->getSize());
}

const luaL_Reg guilib [] = {
    {"get_viewport", lua_wrap_errors<l_gui_getviewport>},
    {"getattr", lua_wrap_errors<l_gui_getattr>},
    {"setattr", lua_wrap_errors<l_gui_setattr>},
    {"get_env", lua_wrap_errors<l_gui_get_env>},
    {"str", lua_wrap_errors<l_gui_str>},
    {"reindex", lua_wrap_errors<l_gui_reindex>},
    {"get_locales_info", lua_wrap_errors<l_gui_get_locales_info>},
    {NULL, NULL}
};
