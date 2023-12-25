#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class EnginePaths;
class Content;
class Level;
class Block;
class Player;
struct block_funcs_set;

namespace scripting {
    extern const Content* content;
    extern Level* level;

    void initialize(EnginePaths* paths);
    void on_world_load(Level* level);
    void on_world_quit();
    void update_block(const Block* block, int x, int y, int z);
    void random_update_block(const Block* block, int x, int y, int z);
    void on_block_placed(Player* player, const Block* block, int x, int y, int z);
    void on_block_broken(Player* player, const Block* block, int x, int y, int z);
    void load_block_script(std::string prefix, fs::path file, block_funcs_set* funcsset);
    void close();
}
