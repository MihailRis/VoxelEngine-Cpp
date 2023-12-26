#include "langs.h"

#include <iostream>

#include "../../coders/json.h"
#include "../../coders/commons.h"
#include "../../content/ContentPack.h"
#include "../../files/files.h"
#include "../../util/stringutil.h"

using std::string;
using std::wstring;
using std::vector;
using std::unique_ptr;
using std::unordered_map;
using std::filesystem::path;
namespace fs = std::filesystem;

unique_ptr<langs::Lang> langs::current;
unordered_map<string, langs::LocaleInfo> langs::locales_info;

langs::Lang::Lang(string locale) : locale(locale) {
}

const wstring& langs::Lang::get(const wstring& key) const  {
    auto found = map.find(key);
    if (found == map.end()) {
        return key;
    }
    return found->second;
}

void langs::Lang::put(const wstring& key, const wstring& text) {
    map[key] = text;
}

const string& langs::Lang::getId() const {
    return locale;
}

/* Language key-value txt files parser */
class Reader : public BasicParser {
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
    Reader(string file, string source) : BasicParser(file, source) {
    }

    void read(langs::Lang& lang, std::string prefix) {
        skipWhitespace();
        while (hasNext()) {
            string key = parseString('=', true);
            util::trim(key);
            key = prefix + key;
            string text = parseString('\n', false);
            util::trim(text);
            lang.put(util::str2wstr_utf8(key),
                     util::str2wstr_utf8(text));
            skipWhitespace();
        }
    }
};

void langs::loadLocalesInfo(const path& resdir, string& fallback) {
    path file = resdir/path(langs::TEXTS_FOLDER)/path("langs.json");
    unique_ptr<json::JObject> root (files::read_json(file));

    langs::locales_info.clear();
    root->str("fallback", fallback);

    auto langs = root->obj("langs");
    if (langs) {
        std::cout << "locales ";
        for (auto& entry : langs->map) {
            auto langInfo = entry.second;

            string name;
            if (langInfo->type == json::valtype::object) {
                name = langInfo->value.obj->getStr("name", "none");
            } else {
                continue;
            }

            std::cout << "[" << entry.first << " (" << name << ")] ";
            langs::locales_info[entry.first] = LocaleInfo {entry.first, name};
        } 
        std::cout << "added" << std::endl;
    }
}

std::string langs::locale_by_envlocale(const std::string& envlocale, const path& resdir){
    string fallback = FALLBACK_DEFAULT;
    if (locales_info.size() == 0) {
        loadLocalesInfo(resdir, fallback);
    }
    if (locales_info.find(envlocale) != locales_info.end()) {
        std::cout << "locale " << envlocale << " is automatically selected" << std::endl;
        return envlocale;
    }
    else {
        for (const auto& loc : locales_info) {
            if (loc.first.find(envlocale.substr(0, 2)) != std::string::npos) {
                std::cout << "locale " << loc.first << " is automatically selected" << std::endl;
                return loc.first;
            }
        }
        std::cout << "locale " << fallback << " is automatically selected" << std::endl;
        return fallback;
    }
}

void langs::load(const path& resdir,
                 const string& locale,
                 const vector<ContentPack>& packs,
                 Lang& lang) {
    path filename = path(TEXTS_FOLDER)/path(locale + LANG_FILE_EXT);
    path core_file = resdir/filename;
    if (fs::is_regular_file(core_file)) {
        string text = files::read_string(core_file);
        Reader reader(core_file.string(), text);
        reader.read(lang, "");
    }
    for (auto pack : packs) {
        path file = pack.folder/filename;
        if (fs::is_regular_file(file)) {
            string text = files::read_string(file);
            Reader reader(file.string(), text);
            reader.read(lang, pack.id+":");
        }
    }
}

void langs::load(const path& resdir,
                 const string& locale,
                 const string& fallback,
                 const vector<ContentPack>& packs) {
    unique_ptr<Lang> lang (new Lang(locale));
    load(resdir, fallback, packs, *lang.get());
    load(resdir, locale, packs, *lang.get());
    current.reset(lang.release());
}

void langs::setup(const path& resdir,
                  const string& locale,
                  const vector<ContentPack>& packs) {
    string fallback = langs::FALLBACK_DEFAULT;
    langs::loadLocalesInfo(resdir, fallback);
    langs::load(resdir, locale, fallback, packs);
}

const wstring& langs::get(const wstring& key) {
    return current->get(key);
}

const wstring& langs::get(const wstring& key, const wstring& context) {
    wstring ctxkey = context + L"." + key;
    const wstring& text = current->get(ctxkey);
    if (&ctxkey != &text) {
        return text;
    }
    return current->get(key);
}
