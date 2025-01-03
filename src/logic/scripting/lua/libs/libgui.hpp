#pragma once

#include <memory>

#include "api_lua.hpp"
#include "frontend/UiDocument.hpp"
#include "graphics/ui/elements/UINode.hpp"

struct DocumentNode {
    UiDocument* document;
    std::shared_ptr<gui::UINode> node;
};

DocumentNode get_document_node(lua::State* L, int idx = 1);
