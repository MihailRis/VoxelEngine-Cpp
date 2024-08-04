#include "PacksManager.hpp"

#include <queue>
#include <sstream>

#include "../util/listutil.hpp"

PacksManager::PacksManager() = default;

void PacksManager::setSources(std::vector<fs::path> sources) {
    this->sources = std::move(sources);
}

void PacksManager::scan() {
    packs.clear();

    std::vector<ContentPack> packsList;
    for (auto& folder : sources) {
        ContentPack::scanFolder(folder, packsList);
        for (auto& pack : packsList) {
            packs.try_emplace(pack.id, pack);
        }
    }
}

void PacksManager::exclude(const std::string& id) {
    packs.erase(id);
}

std::vector<std::string> PacksManager::getAllNames() const {
    std::vector<std::string> names;
    for (auto& entry : packs) {
        names.push_back(entry.first);
    }
    return names;
}

std::vector<ContentPack> PacksManager::getAll(
    const std::vector<std::string>& names
) const {
    std::vector<ContentPack> packsList;
    for (auto& name : names) {
        auto found = packs.find(name);
        if (found == packs.end()) {
            throw contentpack_error(name, fs::path(""), "pack not found");
        }
        packsList.push_back(found->second);
    }
    return packsList;
}

static contentpack_error on_circular_dependency(
    std::queue<const ContentPack*>& queue
) {
    const ContentPack* lastPack = queue.back();
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
/// @param resolveWeaks make weak dependencies resolved if found but not added
/// to queue
/// @return true if all dependencies are already added or not found
/// (optional/weak)
/// @throws contentpack_error if required dependency is not found
static bool resolve_dependencies(
    const ContentPack* pack,
    const std::unordered_map<std::string, ContentPack>& packs,
    std::vector<std::string>& allNames,
    const std::vector<std::string>& added,
    std::queue<const ContentPack*>& queue,
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
            throw contentpack_error(
                dep.id, fs::path(), "dependency of '" + pack->id + "'"
            );
        }
        if (!exists) {
            // ignored for optional or weak dependencies
            continue;
        }
        if (resolveWeaks && dep.level == DependencyLevel::weak) {
            // dependency pack is found but not added yet
            // resolveWeaks is used on second iteration, so it's will not be
            // added
            continue;
        }

        if (!util::contains(allNames, dep.id) &&
            dep.level != DependencyLevel::weak) {
            allNames.push_back(dep.id);
            queue.push(&found->second);
        }
        satisfied = false;
    }
    return satisfied;
}

std::vector<std::string> PacksManager::assembly(
    const std::vector<std::string>& names
) const {
    std::vector<std::string> allNames = names;
    std::vector<std::string> added;
    std::queue<const ContentPack*> queue;
    std::queue<const ContentPack*> queue2;

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

            if (resolve_dependencies(
                    pack, packs, allNames, added, queue, resolveWeaks
                )) {
                if (util::contains(added, pack->id)) {
                    throw contentpack_error(
                        pack->id, pack->folder, "pack duplication"
                    );
                }
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

std::vector<std::string> PacksManager::getNames(
    const std::vector<ContentPack>& packs
) {
    std::vector<std::string> result;
    for (const auto& pack : packs) {
        result.push_back(pack.id);
    }
    return result;
}
