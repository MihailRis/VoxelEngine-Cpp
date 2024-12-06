#pragma once

#include <string>
#include <memory>

struct FontStylesScheme;

// VoxelCore Markdown dialect

namespace markdown {

    template <typename CharT>
    struct Result {
        /// @brief Text with erased markdown
        std::basic_string<CharT> text;
        /// @brief Text styles scheme
        std::unique_ptr<FontStylesScheme> styles;
    };

    Result<char> process(std::string_view source, bool eraseMarkdown);
    Result<wchar_t> process(std::wstring_view source, bool eraseMarkdown);
}
