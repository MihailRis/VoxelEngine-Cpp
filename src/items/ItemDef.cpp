#include "ItemDef.hpp"
#include "../util/stringutil.hpp"

ItemDef::ItemDef(std::string name) : name(name) {
    caption = util::id_to_caption(name);
}
