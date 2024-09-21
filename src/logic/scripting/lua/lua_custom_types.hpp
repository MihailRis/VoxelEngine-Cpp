#pragma once

#include <string>
#include <vector>

#include "lua_commons.hpp"

struct fnl_state;
class Heightmap;
class VoxelStructure;

namespace lua {
    class Userdata {
    public:
        virtual ~Userdata() {};
        virtual const std::string& getTypeName() const = 0;
    };

    class LuaBytearray : public Userdata {
        std::vector<ubyte> buffer;
    public:
        LuaBytearray(size_t capacity);
        LuaBytearray(std::vector<ubyte> buffer);
        virtual ~LuaBytearray();

        const std::string& getTypeName() const override {
            return TYPENAME;
        }
        inline std::vector<ubyte>& data() {
            return buffer;
        }

        static int createMetatable(lua::State*);
        inline static std::string TYPENAME = "Bytearray";
    };
    static_assert(!std::is_abstract<LuaBytearray>());

    class LuaHeightmap : public Userdata {
        std::shared_ptr<Heightmap> map;
        std::unique_ptr<fnl_state> noise;
    public:
        LuaHeightmap(const std::shared_ptr<Heightmap>& map);
        LuaHeightmap(uint width, uint height);

        virtual ~LuaHeightmap();

        uint getWidth() const;

        uint getHeight() const;

        float* getValues();

        const float* getValues() const;

        const std::string& getTypeName() const override {
            return TYPENAME;
        }

        std::shared_ptr<Heightmap> getHeightmap() const {
            return map;
        }

        fnl_state* getNoise() {
            return noise.get();
        }

        void setSeed(int64_t seed);

        static int createMetatable(lua::State*);
        inline static std::string TYPENAME = "Heightmap";
    };
    static_assert(!std::is_abstract<LuaHeightmap>());

    class LuaVoxelStructure : public Userdata {
        std::shared_ptr<VoxelStructure> structure;
    public:
        LuaVoxelStructure(std::shared_ptr<VoxelStructure> structure);

        virtual ~LuaVoxelStructure();

        std::shared_ptr<VoxelStructure> getStructure() const {
            return structure;
        }

        const std::string& getTypeName() const override {
            return TYPENAME;
        }

        static int createMetatable(lua::State*);
        inline static std::string TYPENAME = "VoxelStructure";
    };
    static_assert(!std::is_abstract<LuaVoxelStructure>());
}
