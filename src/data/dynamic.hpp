#ifndef DATA_DYNAMIC_HPP_
#define DATA_DYNAMIC_HPP_

#include "../typedefs.hpp"

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <stdexcept>
#include <unordered_map>

namespace dynamic {
    class Map;
    class List;
    class Value;

    enum class valtype {
        none=0, map, list, string, number, boolean, integer
    };

    using valvalue = std::variant<
        std::monostate,
        Map*,
        List*,
        std::string,
        number_t,
        bool,
        integer_t
    >;

    class Value {
    public:
        valvalue value;
        Value(valvalue value);
        ~Value();

        static std::unique_ptr<Value> boolean(bool value);
        static std::unique_ptr<Value> of(number_u value);
        static std::unique_ptr<Value> of(const std::string& value);
        static std::unique_ptr<Value> of(std::unique_ptr<Map> value);
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

        template<typename T>
        List& put(T value) {
            return put(std::make_unique<Value>(value));
        }

        List& put(std::unique_ptr<Value> value);

        Value* getValueWriteable(size_t index) const;

        List& putList();
        Map& putMap();

        void remove(size_t index);
    };

    class Map {
    public:
        std::unordered_map<std::string, std::unique_ptr<Value>> values;
        ~Map();

        template<typename T>
        T get(const std::string& key) const {
            if (!has(key)) {
                throw std::runtime_error("missing key '"+key+"'");
            }
            return get(key, T());
        }

        std::string get(const std::string& key, const std::string def) const;
        number_t get(const std::string& key, double def) const;
        integer_t get(const std::string& key, integer_t def) const;
        bool get(const std::string& key, bool def) const;

        int get(const std::string& key, int def) const {
            return get(key, static_cast<integer_t>(def));
        }
        uint get(const std::string& key, uint def) const {
            return get(key, static_cast<integer_t>(def));
        }
        uint64_t get(const std::string& key, uint64_t def) const {
            return get(key, static_cast<integer_t>(def));
        }

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

        template<typename T>
        Map& put(std::string key, T value) {
            return put(key, std::make_unique<Value>(value));
        }
        Map& put(std::string key, uint64_t value) {
            return put(key, Value::of(static_cast<integer_t>(value)));
        }

        Map& put(std::string key, std::unique_ptr<Value> value);

        void remove(const std::string& key);

        List& putList(std::string key);
        Map& putMap(std::string key);

        bool has(const std::string& key) const;
        size_t size() const;
    };
}

#endif // DATA_DYNAMIC_HPP_
