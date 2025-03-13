#pragma once

#include <string>
#include <vector>

#include "lua_commons.hpp"

struct fnl_state;
class Heightmap;
class VoxelFragment;
class Texture;
class ImageData;

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
        LuaBytearray(const ubyte* data, size_t size);
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

        const std::shared_ptr<Heightmap>& getHeightmap() const {
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

    class LuaVoxelFragment : public Userdata {
        std::shared_ptr<VoxelFragment> fragment;
    public:
        LuaVoxelFragment(std::shared_ptr<VoxelFragment> fragment);

        virtual ~LuaVoxelFragment();

        std::shared_ptr<VoxelFragment> getFragment() const {
            return fragment;
        }

        const std::string& getTypeName() const override {
            return TYPENAME;
        }

        static int createMetatable(lua::State*);
        inline static std::string TYPENAME = "VoxelFragment";
    };
    static_assert(!std::is_abstract<LuaVoxelFragment>());

    class LuaCanvas : public Userdata {
    public:
        explicit LuaCanvas(
            std::shared_ptr<Texture> inTexture,
            std::shared_ptr<ImageData> inData
        );
        ~LuaCanvas() override = default;

        const std::string& getTypeName() const override {
            return TYPENAME;
        }

        [[nodiscard]] auto& texture() const {
            return *mTexture;
        }

        [[nodiscard]] auto& data() const {
            return *mData;
        }

        [[nodiscard]] bool hasTexture() const {
            return mTexture != nullptr;
        }

        static int createMetatable(lua::State*);
        inline static std::string TYPENAME = "Canvas";
    private:
        std::shared_ptr<Texture> mTexture; // nullable
        std::shared_ptr<ImageData> mData;
    };
    static_assert(!std::is_abstract<LuaCanvas>());
}
