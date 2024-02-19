#include "libgui.h"

#include <iostream>
#include "../scripting.h"
#include "lua_util.h"
#include "LuaState.h"

#include "../../../engine.h"
#include "../../../assets/Assets.h"
#include "../../../frontend/gui/UINode.h"
#include "../../../frontend/gui/controls.h"
#include "../../../frontend/UiDocument.h"
#include "../../../util/stringutil.h"

static gui::UINode* getDocumentNode(lua_State* L, const std::string& name, const std::string& nodeName) {
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        luaL_error(L, "document '%s' not found", name.c_str());
    }
    auto node = doc->get(nodeName);
    if (node == nullptr) {
        luaL_error(L, "document '%s' has no element with id '%s'", name.c_str(), nodeName.c_str());
    }
    return node.get();
}

static bool getattr(lua_State* L, gui::Button* button, const std::string& attr) {
    if (button == nullptr)
        return false;
    if (attr == "text") {
        lua_pushstring(L, util::wstr2str_utf8(button->getText()).c_str());
        return true;
    }
    return false;
}

static bool getattr(lua_State* L, gui::Label* label, const std::string& attr) {
    if (label == nullptr)
        return false;
    if (attr == "text") {
        lua_pushstring(L, util::wstr2str_utf8(label->getText()).c_str());
        return true;
    }
    return false;
}

static bool setattr(lua_State* L, gui::Button* button, const std::string& attr) {
    if (button == nullptr)
        return false;
    if (attr == "text") {
        button->setText(util::str2wstr_utf8(lua_tostring(L, 4)));
        return true;
    }
    return false;
}

static bool setattr(lua_State* L, gui::Label* label, const std::string& attr) {
    if (label == nullptr)
        return false;
    if (attr == "text") {
        label->setText(util::str2wstr_utf8(lua_tostring(L, 4)));
        return true;
    }
    return false;
}

int l_gui_getattr(lua_State* L) {
    auto docname = lua_tostring(L, 1);
    auto element = lua_tostring(L, 2);
    const std::string attr = lua_tostring(L, 3);
    auto node = getDocumentNode(L, docname, element);

    if (attr == "color") {
        return lua::pushcolor_arr(L, node->getColor());
    } else if (attr == "pos") {
        return lua::pushvec2_arr(L, node->getCoord());
    } else if (attr == "size") {
        return lua::pushvec2_arr(L, node->getSize());
    }

    if (getattr(L, dynamic_cast<gui::Button*>(node), attr))
        return 1;
    if (getattr(L, dynamic_cast<gui::Label*>(node), attr))
        return 1;

    return 0;
}

int l_gui_getviewport(lua_State* L) {
    lua::pushvec2_arr(L, scripting::engine->getGUI()->getContainer()->getSize());
    return 1;
}

int l_gui_setattr(lua_State* L) {
    auto docname = lua_tostring(L, 1);
    auto element = lua_tostring(L, 2);
    const std::string attr = lua_tostring(L, 3);

    auto node = getDocumentNode(L, docname, element);
    if (attr == "pos") {
        node->setCoord(lua::tovec2(L, 4));
    } else if (attr == "size") {
        node->setSize(lua::tovec2(L, 4));
    } else {
        if (setattr(L, dynamic_cast<gui::Button*>(node), attr))
            return 0;
        if (setattr(L, dynamic_cast<gui::Label*>(node), attr))
            return 0;
    }
    return 0;
}

int l_gui_get_env(lua_State* L) {
    auto name = lua_tostring(L, 1);
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        luaL_error(L, "document '%s' not found", name);
    }
    lua_getglobal(L, lua::LuaState::envName(doc->getEnvironment()).c_str());
    return 1;
}
