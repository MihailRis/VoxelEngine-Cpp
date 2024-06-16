#ifndef LOGIC_SCRIPTING_LUA_LUA_CUSTOM_TYPES_HPP_
#define LOGIC_SCRIPTING_LUA_LUA_CUSTOM_TYPES_HPP_

#include "lua_commons.hpp"

#include <string>
#include <memory>

namespace lua {
    class Userdata {
    public:
        virtual ~Userdata() {};
        virtual const std::string& getTypeName() const = 0;
    };

    class Bytearray : public Userdata {
        std::unique_ptr<ubyte[]> buffer;
        size_t capacity;
    public:
        Bytearray(size_t capacity);
        virtual ~Bytearray();

        inline ubyte& operator[](size_t index) {
            return buffer[index];
        }

        const std::string& getTypeName() const override {
            return TYPENAME;
        }

        inline size_t size() const {
            return capacity;
        }
        static int createMetatable(lua::State*);
        inline static std::string TYPENAME = "bytearray";
    };
}

#endif // LOGIC_SCRIPTING_LUA_LUA_CUSTOM_TYPES_HPP_
