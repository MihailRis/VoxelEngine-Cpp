#include "lua_commons.h"
#include "api_lua.h"

#include <iostream>
#include "../scripting.h"
#include "lua_util.h"
#include "LuaState.h"

#include "../../../engine.h"
#include "../../../assets/Assets.h"
#include "../../../graphics/ui/gui_util.h"
#include "../../../graphics/ui/elements/UINode.h"
#include "../../../graphics/ui/elements/controls.h"
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

static bool getattr(lua_State* L, gui::TrackBar* bar, const std::string& attr) {
    if (bar == nullptr)
        return false;
    if (attr == "value") {
        lua_pushnumber(L, bar->getValue()); return true;
    } else if (attr == "min") {
        lua_pushnumber(L, bar->getMin()); return true;
    } else if (attr == "max") {
        lua_pushnumber(L, bar->getMax()); 
        return true;
    } else if (attr == "step") {
        lua_pushnumber(L, bar->getStep()); 
        return true;
    } else if (attr == "trackWidth") {
        lua_pushnumber(L, bar->getTrackWidth()); 
        return true;
    } else if (attr == "trackColor") {
        return lua::pushcolor_arr(L, bar->getTrackColor());
    }
    return false;
}

static bool setattr(lua_State* L, gui::TrackBar* bar, const std::string& attr) {
    if (bar == nullptr)
        return false;
    if (attr == "value") {
        bar->setValue(lua_tonumber(L, 4));
        return true;
    } else if (attr == "min") {
        bar->setMin(lua_tonumber(L, 4));
        return true;
    } else if (attr == "max") {
        bar->setMax(lua_tonumber(L, 4));
        return true;
    } else if (attr == "step") {
        bar->setStep(lua_tonumber(L, 4));
        return true;
    } else if (attr == "trackWidth") {
        bar->setTrackWidth(lua_tonumber(L, 4));
        return true;
    } else if (attr == "trackColor") {
        bar->setTrackColor(lua::tocolor(L, 4));
        return true;
    }
    return false;
}

static bool getattr(lua_State* L, gui::Button* button, const std::string& attr) {
    if (button == nullptr)
        return false;
    if (attr == "text") {
        lua_pushstring(L, util::wstr2str_utf8(button->getText()).c_str());
        return true;
    } else if (attr == "pressedColor") {
        return lua::pushcolor_arr(L, button->getPressedColor());
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

static bool getattr(lua_State* L, gui::FullCheckBox* box, const std::string& attr) {
    if (box == nullptr)
        return false;
    if (attr == "checked") {
        lua_pushboolean(L, box->isChecked());
        return true;
    }
    return false;
}

static bool getattr(lua_State* L, gui::TextBox* box, const std::string& attr) {
    if (box == nullptr)
        return false;
    if (attr == "text") {
        lua_pushstring(L, util::wstr2str_utf8(box->getText()).c_str());
        return true;
    } else if (attr == "placeholder") {
        lua_pushstring(L, util::wstr2str_utf8(box->getPlaceholder()).c_str());
        return true;
    }
    return false;
}

static gui::UINode* getDocumentNode(lua_State* L) {
    lua_getfield(L, 1, "docname");
    lua_getfield(L, 1, "name");
    auto docname = lua_tostring(L, -2);
    auto name = lua_tostring(L, -1);
    auto node = getDocumentNode(L, docname, name);
    lua_pop(L, 2);
    return node;
}

static int menu_back(lua_State* L) {
    auto node = getDocumentNode(L);
    auto menu = dynamic_cast<gui::Menu*>(node);
    menu->back();
    return 0;
}

static bool getattr(lua_State* L, gui::Menu* menu, const std::string& attr) {
    if (menu == nullptr)
        return false;
    if (attr == "page") {
        lua_pushstring(L, menu->getCurrent().name.c_str());
        return true;
    } else if (attr == "back") {
        lua_pushcfunction(L, menu_back);
        return true;
    }
    return false;
}

static bool setattr(lua_State* L, gui::FullCheckBox* box, const std::string& attr) {
    if (box == nullptr)
        return false;
    if (attr == "checked") {
        box->setChecked(lua_toboolean(L, 4));
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
    } else if (attr == "pressedColor") {
        button->setPressedColor(lua::tocolor(L, 4));
    }
    return false;
}

static bool setattr(lua_State* L, gui::TextBox* box, const std::string& attr) {
    if (box == nullptr)
        return false;
    if (attr == "text") {
        box->setText(util::str2wstr_utf8(lua_tostring(L, 4)));
        return true;
    } else if (attr == "placeholder") {
        box->setPlaceholder(util::str2wstr_utf8(lua_tostring(L, 4)));
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

static bool setattr(lua_State* L, gui::Menu* menu, const std::string& attr) {
    if (menu == nullptr)
        return false;
    if (attr == "page") {
        auto page = lua_tostring(L, 4);
        if (menu->has(page)) {
            menu->setPage(page);
        } else {
            menu->setPage("404");
        }
        return true;
    }
    return false;
}

static int container_add(lua_State* L) {
    auto node = dynamic_cast<gui::Container*>(getDocumentNode(L));
    auto xmlsrc = lua_tostring(L, 2);
    try {
        node->add(guiutil::create(xmlsrc));
    } catch (const std::exception& err) {
        luaL_error(L, err.what());
    }
    return 0;
}

static bool getattr(lua_State* L, gui::Container* container, const std::string& attr) {
    if (container == nullptr)
        return false;
    
    if (attr == "add") {
        lua_pushcfunction(L, container_add);
        return true;
    }
    return false;
}

static int l_gui_getattr(lua_State* L) {
    auto docname = lua_tostring(L, 1);
    auto element = lua_tostring(L, 2);
    const std::string attr = lua_tostring(L, 3);
    auto node = getDocumentNode(L, docname, element);

    if (attr == "color") {
        return lua::pushcolor_arr(L, node->getColor());
    } else if (attr == "pos") {
        return lua::pushvec2_arr(L, node->getPos());
    } else if (attr == "size") {
        return lua::pushvec2_arr(L, node->getSize());
    } else if (attr == "hoverColor") {
        return lua::pushcolor_arr(L, node->getHoverColor());
    } else if (attr == "interactive") {
        lua_pushboolean(L, node->isInteractive());
        return 1;
    } else if (attr == "visible") {
        lua_pushboolean(L, node->isVisible());
        return 1;
    }

    if (getattr(L, dynamic_cast<gui::Container*>(node), attr))
        return 1;
    if (getattr(L, dynamic_cast<gui::Button*>(node), attr))
        return 1;
    if (getattr(L, dynamic_cast<gui::Label*>(node), attr))
        return 1;
    if (getattr(L, dynamic_cast<gui::TextBox*>(node), attr))
        return 1;
    if (getattr(L, dynamic_cast<gui::TrackBar*>(node), attr))
        return 1;
    if (getattr(L, dynamic_cast<gui::FullCheckBox*>(node), attr))
        return 1;
    if (getattr(L, dynamic_cast<gui::Menu*>(node), attr))
        return 1;

    return 0;
}

static int l_gui_getviewport(lua_State* L) {
    lua::pushvec2_arr(L, scripting::engine->getGUI()->getContainer()->getSize());
    return 1;
}

static int l_gui_setattr(lua_State* L) {
    auto docname = lua_tostring(L, 1);
    auto element = lua_tostring(L, 2);
    const std::string attr = lua_tostring(L, 3);

    auto node = getDocumentNode(L, docname, element);
    if (attr == "pos") {
        node->setPos(lua::tovec2(L, 4));
    } else if (attr == "size") {
        node->setSize(lua::tovec2(L, 4));
    } else if (attr == "color") {
        node->setColor(lua::tocolor(L, 4));
    } else if (attr == "hoverColor") {
        node->setHoverColor(lua::tocolor(L, 4));
    } else if (attr == "interactive") {
        node->setInteractive(lua_toboolean(L, 4));
    } else if (attr == "visible") {
        node->setVisible(lua_toboolean(L, 4));
    } else {
        if (setattr(L, dynamic_cast<gui::Button*>(node), attr))
            return 0;
        if (setattr(L, dynamic_cast<gui::Label*>(node), attr))
            return 0;
        if (setattr(L, dynamic_cast<gui::TextBox*>(node), attr))
            return 0;
        if (setattr(L, dynamic_cast<gui::TrackBar*>(node), attr))
            return 0;
        if (setattr(L, dynamic_cast<gui::FullCheckBox*>(node), attr))
            return 0;
        if (setattr(L, dynamic_cast<gui::Menu*>(node), attr))
            return 0;
    }
    return 0;
}

static int l_gui_get_env(lua_State* L) {
    auto name = lua_tostring(L, 1);
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        luaL_error(L, "document '%s' not found", name);
    }
    lua_getglobal(L, lua::LuaState::envName(doc->getEnvironment()).c_str());
    return 1;
}

const luaL_Reg guilib [] = {
    {"get_viewport", lua_wrap_errors<l_gui_getviewport>},
    {"getattr", lua_wrap_errors<l_gui_getattr>},
    {"setattr", lua_wrap_errors<l_gui_setattr>},
    {"get_env", lua_wrap_errors<l_gui_get_env>},
    {NULL, NULL}
};
