#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include <memory>
#include "../coders/json.h"

class Serializable
{
public:
    virtual ~Serializable() { }
    virtual std::unique_ptr<dynamic::Map>  serialize() const = 0;
    virtual void deserialize(dynamic::Map* src) = 0;
};

#endif