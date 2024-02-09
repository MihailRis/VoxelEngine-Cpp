#include <string>
#include <filesystem>

#include "../../delegates.h"

namespace fs = std::filesystem;

class LuaState;

class Engine;
class Content;
class ContentIndices;
class Level;
class Block;
class Player;
class ItemDef;
struct block_funcs_set;
struct item_funcs_set;
struct uidocscript;
class BlocksController;

namespace scripting {
    extern Engine* engine;
    extern const Content* content;
    extern const ContentIndices* indices;
    extern Level* level;
    extern BlocksController* blocks;

    class Environment {
        int env;
    public:
        Environment(int env);
        ~Environment();

        int getId() const;
    };

    void initialize(Engine* engine);

    runnable create_runnable(
        int env,
        const std::string& filename,
        const std::string& source
    );
    
    wstringconsumer create_wstring_consumer(
        int env,
        const std::string& src,
        const std::string& file="<string>"
    );

    std::unique_ptr<Environment> create_environment();

    void on_world_load(Level* level, BlocksController* blocks);
    void on_world_save();
    void on_world_quit();
    void on_blocks_tick(const Block* block, int tps);
    void update_block(const Block* block, int x, int y, int z);
    void random_update_block(const Block* block, int x, int y, int z);
    void on_block_placed(Player* player, const Block* block, int x, int y, int z);
    void on_block_broken(Player* player, const Block* block, int x, int y, int z);
    bool on_block_interact(Player* player, const Block* block, int x, int y, int z);
    bool on_item_use_on_block(Player* player, const ItemDef* item, int x, int y, int z);
    bool on_item_break_block(Player* player, const ItemDef* item, int x, int y, int z);
    void load_block_script(std::string prefix, fs::path file, block_funcs_set& funcsset);
    void load_item_script(std::string prefix, fs::path file, item_funcs_set& funcsset);
    void load_world_script(std::string prefix, fs::path file);
    void load_layout_script(int env, fs::path file, uidocscript& script);
    void close();
}
