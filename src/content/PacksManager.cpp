#include "PacksManager.h"

#include "../util/listutil.h"

#include <queue>
#include <sstream>

PacksManager::PacksManager() {
}

void PacksManager::setSources(std::vector<fs::path> sources) {
    this->sources = sources;
}

void PacksManager::scan() {
    packs.clear();
    
    std::vector<ContentPack> packsList;
    for (auto& folder : sources) {
        ContentPack::scanFolder(folder, packsList);
        for (auto& pack : packsList) {
            packs.emplace(pack.id, pack);
        }
    }
}

std::vector<std::string> PacksManager::getAllNames() {
    std::vector<std::string> names;
    for (auto& entry : packs) {
        names.push_back(entry.first);
    }
    return names;
}

static contentpack_error on_circular_dependency(std::queue<ContentPack*>& queue) {
    ContentPack* lastPack = queue.back();
    // circular dependency
    std::stringstream ss;
    ss << "circular dependency: " << lastPack->id;
    while (!queue.empty()) {
        auto* pack = queue.front();
        queue.pop();
        ss << " <- " << pack->id;
    }
    return contentpack_error(lastPack->id, lastPack->folder, ss.str());
}

/// @brief Resolve pack dependencies
/// @param pack current pack
/// @param packs all available packs repository
/// @param allNames all already done or enqueued packs
/// @param added packs with all dependencies resolved
/// @param queue current pass queue
/// @param resolveWeaks make weak dependencies resolved if found but not added to queue
/// @return true if all dependencies are already added or not found (optional/weak)
/// @throws contentpack_error if required dependency is not found
static bool resolve_dependencies (
    ContentPack* pack,
    std::unordered_map<std::string, ContentPack>& packs,
    std::vector<std::string>& allNames,
    std::vector<std::string>& added,
    std::queue<ContentPack*>& queue,
    bool resolveWeaks
) {
    bool satisfied = true;
    for (auto& dep : pack->dependencies) {
        if (util::contains(added, dep.id)) {
            continue;
        }
        auto found = packs.find(dep.id);
        bool exists = found != packs.end();
        if (!exists && dep.level == DependencyLevel::required) {
            throw contentpack_error(pack->id, pack->folder, "missing dependency '"+dep.id+"'");
        }
        if (!exists) {
            // ignored for optional or weak dependencies
            continue;
        }
        if (resolveWeaks && dep.level == DependencyLevel::weak) {
            // dependency pack is found but not added yet
            // resolveWeaks is used on second iteration, so it's will not be added
            continue;
        }

        if (!util::contains(allNames, dep.id) && dep.level != DependencyLevel::weak) { 
            allNames.push_back(dep.id);
            queue.push(&found->second);
        }
        satisfied = false;
    }
    return satisfied;
}

std::vector<std::string> PacksManager::assembly(const std::vector<std::string>& names) {
    std::vector<std::string> allNames = names;
    std::vector<std::string> added;
    std::queue<ContentPack*> queue;
    std::queue<ContentPack*> queue2;

    for (auto& name : names) {
        auto found = packs.find(name);
        if (found == packs.end()) {
            throw contentpack_error(name, fs::path(""), "pack not found");
        }
        queue.push(&found->second);
    }

    bool resolveWeaks = false;
    while (!queue.empty()) {
        int addedInIteration = 0;
        while (!queue.empty()) {
            auto* pack = queue.front();
            queue.pop();
            
            if (resolve_dependencies(pack, packs, allNames, added, queue, resolveWeaks)) {
                added.push_back(pack->id);
                addedInIteration++;
            } else {
                queue2.push(pack);
            }
        }
        std::swap(queue, queue2);

        // nothing done but deferring
        if (addedInIteration == 0 && resolveWeaks) {
            throw on_circular_dependency(queue);
        }
        resolveWeaks = true;
    }
    return added;
}
