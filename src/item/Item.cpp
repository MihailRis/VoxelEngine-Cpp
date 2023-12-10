#include "Item.h"

#include <utility>

Item::Item(std::string name) {
    this->name = std::move(name);
}
