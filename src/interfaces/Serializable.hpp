#pragma once

#include "data/dv.hpp"

#include <memory>

class Serializable {
public:
    virtual ~Serializable() {}
    virtual dv::value serialize() const = 0;
    virtual void deserialize(const dv::value& src) = 0;

    void deserializeOpt(const dv::optionalvalue& opt) {
        if (opt.ptr) {
            deserialize(*opt.ptr);
        }
    }
};
