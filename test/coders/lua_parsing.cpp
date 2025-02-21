#include <gtest/gtest.h>

#include "coders/commons.hpp"
#include "coders/lua_parsing.hpp"
#include "files/files.hpp"
#include "util/stringutil.hpp"

TEST(lua_parsing, Tokenizer) {
    auto filename = "../../res/scripts/stdlib.lua";
    auto source = files::read_string(std::filesystem::u8path(filename));
    try {
        auto tokens = lua::tokenize(filename, util::str2wstr_utf8(source));
        for (const auto& token : tokens) {
            std::cout << (int)token.tag << " "
                      << util::quote(util::wstr2str_utf8(token.text))
                      << std::endl;
        }
    } catch (const parsing_error& err) {
        std::cerr << err.errorLog() << std::endl;
        throw err;
    }
}
