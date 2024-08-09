#pragma once

#include <filesystem>
#include <string>

#include "typedefs.hpp"

namespace fs = std::filesystem;

class Hud;

namespace scripting {
    extern Hud *hud;

    void on_frontend_init(Hud *hud);
    void on_frontend_render();
    void on_frontend_close();

    /**
     * Load package-specific hud script
     * @param env environment id
     * @param packid content-pack id
     * @param file script file path
     */
    void load_hud_script(
        const scriptenv &env, const std::string &packid, const fs::path &file
    );
}
