#include "../../data/dynamic.h"
#include "../../typedefs.h"
#include "../../delegates.h"

#include "lua/LuaState.h"
#include "scripting_functional.h"

#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <glm/glm.hpp>

namespace fs = std::filesystem;

class Engine;
class Content;
struct ContentPack;
class ContentIndices;
class Level;
class Block;
class Player;
class ItemDef;
class Inventory;
class UiDocument;
struct block_funcs_set;
struct item_funcs_set;
struct uidocscript;
class BlocksController;
class LevelController;

namespace scripting {
    extern Engine* engine;
    extern const Content* content;
    extern const ContentIndices* indices;
    extern Level* level;
    extern BlocksController* blocks;
    extern LevelController* controller;

    void initialize(Engine* engine);

    extern bool register_event(int env, const std::string& name, const std::string& id);

    static inline int noargs(lua::LuaState *) { return 0; }
    extern bool emit_event(const std::string& name, std::function<int(lua::LuaState* state)> args = noargs);

    scriptenv get_root_environment();
    scriptenv create_pack_environment(const ContentPack& pack);
    scriptenv create_doc_environment(scriptenv parent, const std::string& name);

    void process_post_runnables();

    void on_world_load(LevelController* controller);
    void on_world_tick();
    void on_world_save();
    void on_world_quit();
    void on_blocks_tick(const Block* block, int tps);
    void update_block(const Block* block, int x, int y, int z);
    void random_update_block(const Block* block, int x, int y, int z);
    void on_block_placed(Player* player, const Block* block, int x, int y, int z);
    void on_block_broken(Player* player, const Block* block, int x, int y, int z);
    bool on_block_interact(Player* player, const Block* block, int x, int y, int z);

    /// @brief Called on RMB click with the item selected 
    /// @return true if prevents default action
    bool on_item_use(Player* player, const ItemDef* item);

    /// @brief Called on RMB click on block with the item selected 
    /// @return true if prevents default action
    bool on_item_use_on_block(Player* player, const ItemDef* item, int x, int y, int z);

    /// @brief Called on LMB click on block with the item selected 
    /// @return true if prevents default action
    bool on_item_break_block(Player* player, const ItemDef* item, int x, int y, int z);

    /// @brief Called on UI view show
    void on_ui_open(
        UiDocument* layout, 
        std::vector<std::unique_ptr<dynamic::Value>> args
    );

    /// @brief Called on UI view close
    void on_ui_close(UiDocument* layout, Inventory* inventory);

    /// @brief Load script associated with a Block
    /// @param env environment
    /// @param prefix pack id
    /// @param file item script file
    /// @param funcsset block callbacks set
    void load_block_script(scriptenv env, std::string prefix, fs::path file, block_funcs_set& funcsset);

    /// @brief Load script associated with an Item
    /// @param env environment
    /// @param prefix pack id
    /// @param file item script file
    /// @param funcsset item callbacks set
    void load_item_script(scriptenv env, std::string prefix, fs::path file, item_funcs_set& funcsset);
    
    /// @brief Load package-specific world script 
    /// @param env environment
    /// @param packid content-pack id
    /// @param file script file path
    void load_world_script(scriptenv env, std::string packid, fs::path file);

    /// @brief Load script associated with an UiDocument
    /// @param env environment
    /// @param prefix pack id
    /// @param file item script file
    /// @param script document script info
    void load_layout_script(scriptenv env, std::string prefix, fs::path file, uidocscript& script);

    /// @brief Finalize lua state. Using scripting after will lead to Lua panic
    void close();
}
