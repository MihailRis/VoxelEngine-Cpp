#ifndef CONTENT_CONTENT_LUT_HPP_
#define CONTENT_CONTENT_LUT_HPP_

#include "Content.hpp"

#include "../typedefs.hpp"
#include "../constants.hpp"
#include "../data/dynamic.hpp"

#include <string>
#include <utility>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

struct contententry {
    contenttype type;
    std::string name;
};

template<typename T, class U>
class ContentUnitLUT {
    std::vector<T> indices;
    std::vector<std::string> names;
    bool missingContent = false;
    bool reorderContent = false;
    T missingValue;
    contenttype type;
public:
    ContentUnitLUT(size_t count, const ContentUnitIndices<U>& unitIndices, T missingValue, contenttype type) 
    : missingValue(missingValue), type(type) {
        for (size_t i = 0; i < count; i++) {
            indices.push_back(i);
        }
        for (size_t i = 0; i < unitIndices.count(); i++) {
            names.push_back(unitIndices.get(i)->name);
        }
        for (size_t i = unitIndices.count(); i < count; i++) {
            names.emplace_back("");
        }
    }
    void setup(dynamic::List* list, const ContentUnitDefs<U>& defs) {
        if (list) {
            for (size_t i = 0; i < list->size(); i++) {
                std::string name = list->str(i);
                if (auto def = defs.find(name)) {
                    set(i, name, def->rt.id);
                } else {
                    set(i, name, missingValue);   
                }
            }
        }
    }
    void getMissingContent(std::vector<contententry>& entries) const {
        for (size_t i = 0; i < count(); i++) {
            if (indices[i] == missingValue) {
                auto& name = names[i];
                entries.push_back(contententry {type, name});
            }
        }
    }
    inline const std::string& getName(T index) const {
        return names[index];
    }
    inline T getId(T index) const {
        return indices[index];
    }
    inline void set(T index, std::string name, T id) {
        indices[index] = id;
        names[index] = std::move(name);
        if (id == missingValue) {
            missingContent = true;
        } else if (index != id) {
            reorderContent = true;
        }
    }
    inline size_t count() const {
        return indices.size();
    }
    inline bool hasContentReorder() const {
        return reorderContent;
    }
    inline bool hasMissingContent() const {
        return missingContent;
    }
};

/// @brief Content indices lookup table or report 
/// used to convert world with different indices
/// Building with indices.json
class ContentLUT {
public:
    ContentUnitLUT<blockid_t, Block> blocks;
    ContentUnitLUT<itemid_t, ItemDef> items;

    ContentLUT(const ContentIndices* indices, size_t blocks, size_t items);

    static std::shared_ptr<ContentLUT> create(
        const fs::path& filename, 
        const Content* content
    );
    
    inline bool hasContentReorder() const {
        return blocks.hasContentReorder() || items.hasContentReorder();
    }
    inline bool hasMissingContent() const {
        return blocks.hasMissingContent() || items.hasMissingContent();
    }

    std::vector<contententry> getMissingContent() const;
};

#endif // CONTENT_CONTENT_LUT_HPP_
