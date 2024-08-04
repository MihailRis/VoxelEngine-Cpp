#include "locale.hpp"

#include <utility>

#include "../coders/json.hpp"
#include "../coders/commons.hpp"
#include "../content/ContentPack.hpp"
#include "../files/files.hpp"
#include "../util/stringutil.hpp"
#include "../data/dynamic.hpp"
#include "../debug/Logger.hpp"

static debug::Logger logger("locale");

namespace fs = std::filesystem;
using namespace std::literals;

std::unique_ptr<langs::Lang> langs::current;
std::unordered_map<std::string, langs::LocaleInfo> langs::locales_info;

langs::Lang::Lang(std::string locale) : locale(std::move(locale)) {
}

const std::wstring& langs::Lang::get(const std::wstring& key) const  {
    auto found = map.find(key);
    if (found == map.end()) {
        return key;
    }
    return found->second;
}

void langs::Lang::put(const std::wstring& key, const std::wstring& text) {
    map[key] = text;
}

const std::string& langs::Lang::getId() const {
    return locale;
}

// @brief Language key-value txt files parser
class Reader : BasicParser {
    void skipWhitespace() override {
        BasicParser::skipWhitespace();
        if (hasNext() && source[pos] == '#') {
            skipLine();
            if (hasNext() && is_whitespace(peek())) {
                skipWhitespace();
            }
        }
    }
public:
    Reader(std::string_view file, std::string_view source) : BasicParser(file, source) {
    }

    void read(langs::Lang& lang, const std::string &prefix) {
        skipWhitespace();
        while (hasNext()) {
            std::string key = parseString('=', true);
            util::trim(key);
            key = prefix + key;
            std::string text = parseString('\n', false);
            util::trim(text);
            lang.put(util::str2wstr_utf8(key),
                     util::str2wstr_utf8(text));
            skipWhitespace();
        }
    }
};

void langs::loadLocalesInfo(const fs::path& resdir, std::string& fallback) {
    fs::path file = resdir/fs::path(langs::TEXTS_FOLDER)/fs::path("langs.json");
    auto root = files::read_json(file);

    langs::locales_info.clear();
    root->str("fallback", fallback);

    auto langs = root->map("langs");
    if (langs) {
        auto logline = logger.info();
        logline << "locales ";
        for (auto& entry : langs->values) {
            auto langInfo = entry.second;

            std::string name;
            if (auto mapptr = std::get_if<dynamic::Map_sptr>(&langInfo)) {
                name = (*mapptr)->get("name", "none"s);
            } else {
                continue;
            }
            logline << "[" << entry.first << " (" << name << ")] ";
            langs::locales_info[entry.first] = LocaleInfo {entry.first, name};
        } 
        logline << "added";
    }
}

std::string langs::locale_by_envlocale(const std::string& envlocale, const fs::path& resdir){
    std::string fallback = FALLBACK_DEFAULT;
    if (locales_info.size() == 0) {
        loadLocalesInfo(resdir, fallback);
    }
    if (locales_info.find(envlocale) != locales_info.end()) {
        logger.info() << "locale " << envlocale << " is automatically selected";
        return envlocale;
    }
    else {
        for (const auto& loc : locales_info) {
            if (loc.first.find(envlocale.substr(0, 2)) != std::string::npos) {
                logger.info() << "locale " << loc.first << " is automatically selected";
                return loc.first;
            }
        }
        logger.info() << "locale " << fallback << " is automatically selected";
        return fallback;
    }
}

void langs::load(const fs::path& resdir,
                 const std::string& locale,
                 const std::vector<ContentPack>& packs,
                 Lang& lang) {
    fs::path filename = fs::path(TEXTS_FOLDER)/fs::path(locale + LANG_FILE_EXT);
    fs::path core_file = resdir/filename;
    
    if (fs::is_regular_file(core_file)) {
        std::string text = files::read_string(core_file);
        Reader reader(core_file.string(), text);
        reader.read(lang, "");
    }
    for (auto pack : packs) {
        fs::path file = pack.folder/filename;
        if (fs::is_regular_file(file)) {
            std::string text = files::read_string(file);
            Reader reader(file.string(), text);
            reader.read(lang, "");
        }
    }
}

void langs::load(const fs::path& resdir,
                 const std::string& locale,
                 const std::string& fallback,
                 const std::vector<ContentPack>& packs) {
    auto lang = std::make_unique<Lang>(locale);
    load(resdir, fallback, packs, *lang.get());
    if (locale != fallback) {
        load(resdir, locale, packs, *lang.get());
    }
    current = std::move(lang);
}

void langs::setup(const fs::path& resdir,
                  std::string locale,
                  const std::vector<ContentPack>& packs) {
    std::string fallback = langs::FALLBACK_DEFAULT;
    langs::loadLocalesInfo(resdir, fallback);
    if (langs::locales_info.find(locale) == langs::locales_info.end()) {
        locale = fallback;
    }
    langs::load(resdir, locale, fallback, packs);
}

const std::wstring& langs::get(const std::wstring& key) {
    return current->get(key);
}

const std::wstring& langs::get(const std::wstring& key, const std::wstring& context) {
    std::wstring ctxkey = context + L"." + key;
    const std::wstring& text = current->get(ctxkey);
    if (&ctxkey != &text) {
        return text;
    }
    return current->get(key);
}
