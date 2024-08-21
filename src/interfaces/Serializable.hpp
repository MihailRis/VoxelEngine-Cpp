#pragma once

#include "data/dynamic_fwd.hpp"

#include <memory>

class Serializable {
public:
    virtual ~Serializable() { }
    virtual std::unique_ptr<dynamic::Map>  serialize() const = 0;
    virtual void deserialize(dynamic::Map* src) = 0;
};
