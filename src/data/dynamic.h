#ifndef DATA_DYNAMIC_H_
#define DATA_DYNAMIC_H_

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <unordered_map>
#include "../typedefs.h"

namespace dynamic {
    class Map;
    class List;
    class Value;

    using number_t = double;
    using integer_t = int64_t;

    enum class valtype {
        none, map, list, number, integer, string, boolean
    };

    using valvalue = std::variant<
        Map*,
        List*,
        std::string,
        number_t,
        bool,
        integer_t
    >;

    class Value {
    public:
        valtype type;
        valvalue value;
        Value(valtype type, valvalue value);
        ~Value();
    };

    class List {
    public:
        std::vector<std::unique_ptr<Value>> values;
        ~List();

        std::string str(size_t index) const;
        number_t num(size_t index) const;
        integer_t integer(size_t index) const;
        Map* map(size_t index) const;
        List* list(size_t index) const;
        bool flag(size_t index) const;

        inline size_t size() const {
            return values.size();
        }

        inline Value* get(size_t i) const {
            return values.at(i).get();
        }

        List& put(uint value);
        List& put(int value);
        List& put(uint64_t value);
        List& put(int64_t value);
        List& put(float value);
        List& put(double value);
        List& put(std::string value);
        List& put(Map* value);
        List& put(List* value);
        List& put(bool value);

        Value* getValueWriteable(size_t index) const;

        List& putList();
        Map& putMap();

        void remove(size_t index);
    };

    class Map {
    public:
        std::unordered_map<std::string, std::unique_ptr<Value>> values;
        ~Map();

        std::string getStr(std::string key) const;
        number_t getNum(std::string key) const;
        integer_t getInt(std::string key) const;
        bool getBool(std::string key) const;

        std::string getStr(std::string key, const std::string& def) const;
        number_t getNum(std::string key, double def) const;
        integer_t getInt(std::string key, integer_t def) const;
        bool getBool(std::string key, bool def) const;

        void str(std::string key, std::string& dst) const;
        void num(std::string key, int& dst) const;
        void num(std::string key, float& dst) const;
        void num(std::string key, uint& dst) const;
        void num(std::string key, int64_t& dst) const;
        void num(std::string key, uint64_t& dst) const;
        void num(std::string key, ubyte& dst) const;
        void num(std::string key, double& dst) const;
        Map* map(std::string key) const;
        List* list(std::string key) const;
        void flag(std::string key, bool& dst) const;

        Map& put(std::string key, uint value);
        Map& put(std::string key, int value);
        Map& put(std::string key, int64_t value);
        Map& put(std::string key, uint64_t value);
        Map& put(std::string key, float value);
        Map& put(std::string key, double value);
        Map& put(std::string key, const char* value);
        Map& put(std::string key, std::string value);
        Map& put(std::string key, Map* value);
        Map& put(std::string key, List* value);
        Map& put(std::string key, bool value);

        List& putList(std::string key);
        Map& putMap(std::string key);

        bool has(std::string key);
    };
}

#endif // DATA_DYNAMIC_H_
