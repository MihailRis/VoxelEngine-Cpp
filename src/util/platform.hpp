#pragma once

#include <string>
#include <filesystem>

namespace platform {
    void configure_encoding();
    /// @return environment locale in ISO format ll_CC
    std::string detect_locale();
    /// @brief Open folder using system file manager application
    /// @param folder target folder
    void open_folder(const std::filesystem::path& folder);
}
