#ifndef CODERS_JSON_H_
#define CODERS_JSON_H_

#include <vector>
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>

#include "commons.h"
#include "../typedefs.h"

#include "binary_json.h"

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

#endif // CODERS_JSON_H_
