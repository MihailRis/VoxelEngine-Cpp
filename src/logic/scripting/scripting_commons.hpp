#pragma once

#include <string>
#include <filesystem>

namespace scripting {
    void load_script(const std::filesystem::path& name, bool throwable);

    [[nodiscard]] int load_script(
        int env,
        const std::string& type,
        const std::filesystem::path& file,
        const std::string& fileName
    );
}
