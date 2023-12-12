#ifndef FRONTEND_LOCALE_LANGS_H
#define FRONTEND_LOCALE_LANGS_H

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <unordered_map>

class ContentPack;

namespace langs {
    const char LANG_FILE_EXT[] = ".txt";
    const char TEXTS_FOLDER[] = "texts";
    const char FALLBACK_DEFAULT[] = "en_US";

    /*
        Translation is mostly used for rendered text,
        so Font.draw and Lang both use wstring for strings.
        
        Translation key is wstring too, allowing to use it as value 
        if no any translation found, without conversion required

        Key syntax: `modid:context.key` where modid is added at runtime for
        content pack texts
     */
    class Lang {
        std::string locale;
        std::unordered_map<std::wstring, std::wstring> map;
    public:
        Lang(std::string locale);

        const std::wstring& get(const std::wstring& key) const;
        void put(const std::wstring& key, const std::wstring& text);
    };

    struct LocaleInfo {
        std::string locale;
        std::string name;
    };

    extern std::unique_ptr<Lang> current;
    extern std::vector<LocaleInfo> locales_info;

    extern void loadLocalesInfo(
        const std::filesystem::path& resdir,
        std::string& fallback);

    extern void load(const std::filesystem::path& resdir,
                     const std::string& locale,
                     std::vector<const ContentPack*>& packs,
                     Lang& lang);
    extern void load(const std::filesystem::path& resdir,
                     const std::string& locale,
                     const std::string& fallback,
                     std::vector<const ContentPack*>& packs);

    extern const std::wstring& get(const std::wstring& key);
    extern const std::wstring& get(const std::wstring& key, 
                                   const std::wstring& context);

    extern void setup(const std::filesystem::path& resdir,
                      const std::string& locale,
                      std::vector<const ContentPack*>& packs);
}

#endif // FRONTEND_LOCALE_LANGS_H
