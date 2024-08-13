#pragma once

#include <string>
#include <vector>

#include "lua_commons.hpp"

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
        inline static std::string TYPENAME = "bytearray";
    };

    class Heightmap : public Userdata {
        std::vector<float> buffer;
        uint width, height;
    public:
        Heightmap(uint width, uint height);
        virtual ~Heightmap();

        uint getWidth() const {
            return width;
        }

        uint getHeight() const {
            return height;
        }

        const std::string& getTypeName() const override {
            return TYPENAME;
        }

        float* getValues() {
            return buffer.data();
        }

        const float* getValues() const {
            return buffer.data();
        }

        static int createMetatable(lua::State*);
        inline static std::string TYPENAME = "heightmap";
    };
}
