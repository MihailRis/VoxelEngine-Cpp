#ifndef CONTENT_CONTENT_INDEX_LUT_H_
#define CONTENT_CONTENT_INDEX_LUT_H_

#include "../typedefs.h"
#include <string>
#include <filesystem>

class Content;

/* Content indices lookup table or report 
   used to convert world with different indices
   Building with indices.json */
class ContentIndexLUT {
    blockid_t* blocks;
    std::string* blockNames;
public:
    ContentIndexLUT(size_t blocks, const Content* content);
    ~ContentIndexLUT();

    inline blockid_t getBlockId(blockid_t index) {
        return blocks[index];
    }

    inline void setBlock(blockid_t index, std::string name, blockid_t id) {
        blocks[index] = id;
        blockNames[index] = name;
    }

    static ContentIndexLUT* create(const std::filesystem::path& filename, 
                                   const Content* content);
};

#endif // CONTENT_CONTENT_INDEX_LUT_H_
