#include "markdown.hpp"

#include <sstream>

#include "graphics/core/Font.hpp"

using namespace markdown;

template <typename CharT>
static inline void emit(
    CharT c, FontStylesScheme& styles, std::basic_stringstream<CharT>& ss
) {
    ss << c;
    styles.map.emplace_back(styles.palette.size()-1);
}

template <typename CharT>
static inline void emit_md(
    CharT c, FontStylesScheme& styles, std::basic_stringstream<CharT>& ss
) {
    ss << c;
    styles.map.emplace_back(0);
}

template <typename CharT>
static inline void restyle(
    CharT c,
    FontStyle& style,
    FontStylesScheme& styles,
    std::basic_stringstream<CharT>& ss,
    int& pos,
    bool eraseMarkdown
) {
    styles.palette.push_back(style);
    if (!eraseMarkdown) {
        emit_md(c, styles, ss);
    }
    pos++;
}

template <typename CharT>
Result<CharT> process_markdown(
    std::basic_string_view<CharT> source, bool eraseMarkdown
) {
    std::basic_stringstream<CharT> ss;
    FontStylesScheme styles {
        // markdown                                           default
        {{false, false, false, false, glm::vec4(1,1,1,0.5f)}, {}},
        {}
    };
    FontStyle style;
    int pos = 0;
    while (pos < source.size()) {
        CharT first = source[pos];
        if (first == '\\') {
            if (pos + 1 < source.size()) {
                CharT second = source[++pos];
                switch (second) {
                    case '*':
                    case '_':
                    case '~':
                        if (!eraseMarkdown) {
                            emit_md(first, styles, ss);
                        }
                        emit(second, styles, ss);
                        pos++;
                        continue;
                }
            }
        } else if (first == '*') {
            if (pos + 1 < source.size() && source[pos+1] == '*') {
                pos++;
                if (!eraseMarkdown)
                    emit_md(first, styles, ss);
                style.bold = !style.bold;
                restyle(first, style, styles, ss, pos, eraseMarkdown);
                continue;
            }
            style.italic = !style.italic;
            restyle(first, style, styles, ss, pos, eraseMarkdown);
            continue;
        } else if (first == '_' && pos + 1 < source.size() && source[pos+1] == '_') {
            pos++;
            if (!eraseMarkdown)
                emit_md(first, styles, ss);
            style.underline = !style.underline;
            restyle(first, style, styles, ss, pos, eraseMarkdown);
            continue;
        } else if (first == '~' && pos + 1 < source.size() && source[pos+1] == '~') {
            pos++;
            if (!eraseMarkdown)
                emit_md(first, styles, ss);
            style.strikethrough = !style.strikethrough;
            restyle(first, style, styles, ss, pos, eraseMarkdown);
            continue;
        }
        emit(first, styles, ss);
        pos++;
    }
    return {ss.str(), std::make_unique<FontStylesScheme>(std::move(styles))};
}

Result<char> markdown::process(std::string_view source, bool eraseMarkdown) {
    return process_markdown(source, eraseMarkdown);
}

Result<wchar_t> markdown::process(std::wstring_view source, bool eraseMarkdown) {
    return process_markdown(source, eraseMarkdown);
}
