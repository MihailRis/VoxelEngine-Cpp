#pragma once

#include <string>
#include <vector>

#include "lua_commons.hpp"
#include "maths/Heightmap.hpp"

namespace lua {
    class Userdata {
    public:
        virtual ~Userdata() {};
        virtual const std::string& getTypeName() const = 0;
    };

    class Bytearray : public Userdata {
        std::vector<ubyte> buffer;
    public:
        Bytearray(size_t capacity);
        Bytearray(std::vector<ubyte> buffer);
        virtual ~Bytearray();

        const std::string& getTypeName() const override {
            return TYPENAME;
        }
        inline std::vector<ubyte>& data() {
            return buffer;
        }

        static int createMetatable(lua::State*);
        inline static std::string TYPENAME = "Bytearray";
    };

    class LuaHeightmap : public Userdata {
        std::shared_ptr<Heightmap> map;
    public:
        LuaHeightmap(uint width, uint height) 
        : map(std::make_shared<Heightmap>(width, height)) {}

        virtual ~LuaHeightmap() = default;

        uint getWidth() const {
            return map->getWidth();
        }

        uint getHeight() const {
            return map->getHeight();
        }

        float* getValues() {
            return map->getValues();
        }

        const float* getValues() const {
            return map->getValues();
        }

        const std::string& getTypeName() const override {
            return TYPENAME;
        }

        std::shared_ptr<Heightmap> getHeightmap() const {
            return map;
        }

        static int createMetatable(lua::State*);
        inline static std::string TYPENAME = "Heightmap";
    };
}
