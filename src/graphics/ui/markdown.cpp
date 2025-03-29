#include "markdown.hpp"

#include "coders/commons.hpp"
#include "graphics/core/Font.hpp"

using namespace markdown;

template <typename CharT>
static inline void emit(
    CharT c, FontStylesScheme& styles, std::basic_stringstream<CharT>& ss
) {
    ss << c;
    styles.map.emplace_back(styles.palette.size() - 1);
}

template <typename CharT>
static inline void emit_md(
    CharT c, FontStylesScheme& styles, std::basic_stringstream<CharT>& ss
) {
    ss << c;
    styles.map.emplace_back(0);
}

template <typename CharT>
static glm::vec4 parse_color(const std::basic_string_view<CharT>& color_code) {
    auto hex_to_float = [](char high, char low) {
        int high_val = hexchar2int(high);
        int low_val = hexchar2int(low);
        if (high_val == -1 || low_val == -1) {
            return 1.0f;  // default to max value on error
        }
        return (high_val * 16 + low_val) / 255.0f;
    };

    if (color_code[0] != '#') {
        return glm::vec4(1, 1, 1, 1);
    }

    if (color_code.size() < 8) {
        return glm::vec4(1, 1, 1, 1);
    }
    return glm::vec4(
        hex_to_float(color_code[1], color_code[2]),
        hex_to_float(color_code[3], color_code[4]),
        hex_to_float(color_code[5], color_code[6]),
        color_code.size() == 10 ? hex_to_float(color_code[7], color_code[8]) : 1
    );
}

template <typename CharT>
static inline void apply_color(
    const std::basic_string_view<CharT>& color_code, 
    FontStylesScheme& styles,
    FontStyle& style
) {
    style.color = parse_color(color_code);
    styles.palette.push_back(style);
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
        {{false, false, false, false, glm::vec4(1, 1, 1, 0.5f)}, {}},
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
                    case '[':
                        if (source[pos + 1] == '#') {
                            int closingPos = -1;
                            if (pos + 8 < source.size() && source[pos + 8] == ']') {
                                closingPos = 8;
                            } else if (pos + 10 < source.size() && source[pos + 10] == ']') {
                                closingPos = 10;
                            }
                            if (closingPos != -1) {
                                if (!eraseMarkdown) {
                                    emit_md(source[pos - 1], styles, ss);
                                }
                                int length = closingPos + 2;
                                for (int i = 0; i < length; ++i) {
                                    emit(source[pos + i], styles, ss);
                                }
                                pos += length;
                                continue;
                            }
                        }
                }
                pos--;
            }
        } else if (first == '[' && pos + 9 <= source.size() && source[pos + 1] == '#' && source[pos + 8] == ']') {
            std::basic_string_view<CharT> color_code = source.substr(pos + 1, 8);
            apply_color(color_code, styles, style);
            if (!eraseMarkdown) {
                for (int i = 0; i < 9; ++i) {
                    emit_md(source[pos + i], styles, ss);
                }
            }
            pos += 9;  // Skip past the color code
            continue;
        } else if (first == '[' && pos + 11 <= source.size() && source[pos + 1] == '#' && source[pos + 10] == ']') {
            std::basic_string_view<CharT> color_code = source.substr(pos + 1, 10);
            apply_color(color_code, styles, style);
            if (!eraseMarkdown) {
                for (int i = 0; i < 11; ++i) {
                    emit_md(source[pos + i], styles, ss);
                }
            }
            pos += 11;  // Skip past the color code
            continue;
        } else if (first == '*') {
            if (pos + 1 < source.size() && source[pos + 1] == '*') {
                pos++;
                if (!eraseMarkdown) emit_md(first, styles, ss);
                style.bold = !style.bold;
                restyle(first, style, styles, ss, pos, eraseMarkdown);
                continue;
            }
            style.italic = !style.italic;
            restyle(first, style, styles, ss, pos, eraseMarkdown);
            continue;
        } else if (first == '_' && pos + 1 < source.size() &&
                   source[pos + 1] == '_') {
            pos++;
            if (!eraseMarkdown) emit_md(first, styles, ss);
            style.underline = !style.underline;
            restyle(first, style, styles, ss, pos, eraseMarkdown);
            continue;
        } else if (first == '~' && pos + 1 < source.size() &&
                   source[pos + 1] == '~') {
            pos++;
            if (!eraseMarkdown) emit_md(first, styles, ss);
            style.strikethrough = !style.strikethrough;
            restyle(first, style, styles, ss, pos, eraseMarkdown);
            continue;
        }
        if (first == '\n') {
            styles.palette.push_back(styles.palette.at(1));
        }
        emit(first, styles, ss);
        pos++;
    }
    return {ss.str(), std::make_unique<FontStylesScheme>(std::move(styles))};
}

Result<char> markdown::process(std::string_view source, bool eraseMarkdown) {
    return process_markdown(source, eraseMarkdown);
}

Result<wchar_t> markdown::process(
    std::wstring_view source, bool eraseMarkdown
) {
    return process_markdown(source, eraseMarkdown);
}
