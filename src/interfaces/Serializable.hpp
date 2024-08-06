#ifndef SERIALIZABLE_HPP_
#define SERIALIZABLE_HPP_

#include <coders/json.hpp>

#include <memory>

class Serializable {
public:
    virtual ~Serializable() { }
    virtual std::unique_ptr<dynamic::Map>  serialize() const = 0;
    virtual void deserialize(dynamic::Map* src) = 0;
};

#endif // SERIALIZABLE_HPP_
