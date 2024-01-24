#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class Engine;
class Content;
class Level;
class Block;
class Player;
class ItemDef;
struct block_funcs_set;
struct item_funcs_set;
class BlocksController;

namespace scripting {
    extern Engine* engine;
    extern const Content* content;
    extern Level* level;
    extern BlocksController* blocks;

    void initialize(Engine* engine);
    void on_world_load(Level* level, BlocksController* blocks);
    void on_world_quit();
    void on_blocks_tick(const Block* block, int tps);
    void update_block(const Block* block, int x, int y, int z);
    void random_update_block(const Block* block, int x, int y, int z);
    void on_block_placed(Player* player, const Block* block, int x, int y, int z);
    void on_block_broken(Player* player, const Block* block, int x, int y, int z);
    void on_block_interact(Player* player, const Block* block, int x, int y, int z);
    bool on_item_use_on_block(Player* player, const ItemDef* item, int x, int y, int z);
    bool on_item_break_block(Player* player, const ItemDef* item, int x, int y, int z);
    void load_block_script(std::string prefix, fs::path file, block_funcs_set* funcsset);
    void load_item_script(std::string prefix, fs::path file, item_funcs_set* funcsset);
    void close();
}
