#pragma once

#include "graphics/commons/Model.hpp"

struct ItemDef;
class Assets;
class Content;

class ModelsGenerator {
public:
    static model::Model generate(
        const ItemDef& def, const Content& content, const Assets& assets
    );
};
