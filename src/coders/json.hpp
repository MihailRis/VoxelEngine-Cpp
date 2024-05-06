#ifndef CODERS_JSON_HPP_
#define CODERS_JSON_HPP_

#include "commons.hpp"
#include "binary_json.hpp"

#include "../typedefs.hpp"

#include <vector>
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>

namespace dynamic {
    class Map;
    class List;
    class Value;
}

namespace json {
    class Parser : public BasicParser {
        dynamic::List* parseList();
        dynamic::Map* parseObject();
        dynamic::Value* parseValue();
    public:
        Parser(const std::string& filename, const std::string& source);
        
        dynamic::Map* parse();
    };

    extern std::unique_ptr<dynamic::Map> parse(const std::string& filename, const std::string& source);
    extern std::unique_ptr<dynamic::Map> parse(const std::string& source);

    extern std::string stringify(
        const dynamic::Map* obj, 
        bool nice, 
        const std::string& indent);
}

#endif // CODERS_JSON_HPP_
