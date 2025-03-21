#include "ContentControl.hpp"

#include "io/io.hpp"
#include "io/engine_paths.hpp"
#include "coders/toml.hpp"
#include "Content.hpp"
#include "ContentPack.hpp"
#include "ContentBuilder.hpp"
#include "ContentLoader.hpp"
#include "PacksManager.hpp"
#include "objects/rigging.hpp"
#include "logic/scripting/scripting.hpp"
#include "window/input.hpp"
#include "core_defs.hpp"

static void load_configs(Input& input, const io::path& root) {
    auto configFolder = root / "config";
    auto bindsFile = configFolder / "bindings.toml";
    if (io::is_regular_file(bindsFile)) {
        input.getBindings().read(
            toml::parse(bindsFile.string(), io::read_string(bindsFile)),
            BindType::BIND
        );
    }
}

ContentControl::ContentControl(std::function<void()> postContent)
    : postContent(std::move(postContent)) {
    basePacks = io::read_list("res:config/builtins.list");
}

ContentControl::~ContentControl() = default;

Content* ContentControl::get() {
    return content.get();
}

std::vector<std::string>& ContentControl::getBasePacks() {
    return basePacks;
}

void ContentControl::resetContent(
    EnginePaths& paths, Input& input, std::vector<ContentPack>& contentPacks
) {
    scripting::cleanup();
    std::vector<PathsRoot> resRoots;
    {
        auto pack = ContentPack::createCore(paths);
        resRoots.push_back({"core", pack.folder});
        load_configs(input, pack.folder);
    }
    PacksManager manager;
    manager.setSources({
        "user:content",
        "res:content",
    });
    manager.scan();
    for (const auto& pack : manager.getAll(basePacks)) {
        resRoots.push_back({pack.id, pack.folder});
    }
    paths.resPaths = ResPaths(resRoots);
    content.reset();

    contentPacks.clear();
    contentPacks = manager.getAll(basePacks);

    postContent();
}

void ContentControl::loadContent(
    EnginePaths& paths,
    Input& input,
    std::vector<ContentPack>& packs,
    const std::vector<std::string>& names
) {
    PacksManager manager;
    manager.setSources(getDefaultSources());
    manager.scan();
    packs = manager.getAll(manager.assemble(names));
    loadContent(paths, input, packs);
}

void ContentControl::loadContent(
    EnginePaths& paths,
    Input& input,
    std::vector<ContentPack>& contentPacks
) {
    scripting::cleanup();

    std::vector<std::string> names;
    for (auto& pack : contentPacks) {
        names.push_back(pack.id);
    }

    PacksManager manager;
    manager.setSources(getDefaultSources());
    manager.scan();
    names = manager.assemble(names);
    contentPacks = manager.getAll(names);

    std::vector<PathsRoot> entryPoints;
    for (auto& pack : contentPacks) {
        entryPoints.emplace_back(pack.id, pack.folder);
    }
    paths.setEntryPoints(std::move(entryPoints));

    ContentBuilder contentBuilder;
    corecontent::setup(input, contentBuilder);

    auto corePack = ContentPack::createCore(paths);

    auto allPacks = contentPacks;
    allPacks.insert(allPacks.begin(), corePack);

    // Setup filesystem entry points
    std::vector<PathsRoot> resRoots;
    for (auto& pack : allPacks) {
        resRoots.push_back({pack.id, pack.folder});
    }
    paths.resPaths = ResPaths(resRoots);
    // Load content
    for (auto& pack : allPacks) {
        ContentLoader(&pack, contentBuilder, paths.resPaths).load();
        load_configs(input, pack.folder);
    }
    content = contentBuilder.build();
    scripting::on_content_load(content.get());

    ContentLoader::loadScripts(*content);

    postContent();
}

std::vector<io::path> ContentControl::getDefaultSources() {
    return {
        "world:content",
        "user:content",
        "res:content",
    };
}
