#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

#include "typedefs.hpp"

namespace fs = std::filesystem;

class Hud;
class WorldRenderer;

namespace gui {
    class UINode;
    using PageLoaderFunc = std::function<std::shared_ptr<UINode>(const std::string&)>;
}

namespace scripting {
    extern Hud *hud;
    extern WorldRenderer* renderer;

    void on_frontend_init(Hud* hud, WorldRenderer* renderer);
    void on_frontend_render();
    void on_frontend_close();

    /// @brief Load package-specific hud script
    /// @param env environment id
    /// @param packid content-pack id
    /// @param file script file path
    /// @param fileName script file path using the engine format
    void load_hud_script(
        const scriptenv& env,
        const std::string& packid,
        const fs::path& file,
        const std::string& fileName
    );

    gui::PageLoaderFunc create_page_loader(); 
}
