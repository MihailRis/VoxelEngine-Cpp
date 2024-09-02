#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "constants.hpp"
#include "data/dynamic.hpp"
#include "typedefs.hpp"
#include "Content.hpp"

namespace fs = std::filesystem;

enum class ContentIssueType {
    REORDER,
    MISSING,
};

struct ContentIssue {
    ContentIssueType issueType;
    ContentType contentType;
};

struct ContentEntry {
    ContentType type;
    std::string name;
};

class WorldFiles;

template <typename T, class U>
class ContentUnitLUT {
    std::vector<T> indices;
    std::vector<std::string> names;
    bool missingContent = false;
    bool reorderContent = false;
    T missingValue;
    ContentType type;
public:
    ContentUnitLUT(
        size_t count,
        const ContentUnitIndices<U>& unitIndices,
        T missingValue,
        ContentType type
    )
        : missingValue(missingValue), type(type) {
        for (size_t i = 0; i < count; i++) {
            indices.push_back(i);
        }
        for (auto unit : unitIndices.getIterable()) {
            names.push_back(unit->name); 
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
    void getMissingContent(std::vector<ContentEntry>& entries) const {
        for (size_t i = 0; i < count(); i++) {
            if (indices[i] == missingValue) {
                auto& name = names[i];
                entries.push_back(ContentEntry {type, name});
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
    inline ContentType getContentType() const {
        return type;
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

/// @brief Content incapatibility report used to convert world.
/// Building with indices.json
class ContentReport {
public:
    ContentUnitLUT<blockid_t, Block> blocks;
    ContentUnitLUT<itemid_t, ItemDef> items;

    std::vector<ContentIssue> issues;

    ContentReport(
        const ContentIndices* indices, 
        size_t blocks, 
        size_t items
    );

    static std::shared_ptr<ContentReport> create(
        const std::shared_ptr<WorldFiles>& worldFiles,
        const fs::path& filename,
        const Content* content
    );

    inline bool hasContentReorder() const {
        return blocks.hasContentReorder() || items.hasContentReorder();
    }
    inline bool hasMissingContent() const {
        return blocks.hasMissingContent() || items.hasMissingContent();
    }
    void buildIssues();

    const std::vector<ContentIssue>& getIssues() const;
    std::vector<ContentEntry> getMissingContent() const;
};
