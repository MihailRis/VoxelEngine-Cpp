#ifndef DATA_DYNAMIC_HPP_
#define DATA_DYNAMIC_HPP_

#include "dynamic_fwd.hpp"

#include <cmath>
#include <string>
#include <vector>
#include <memory>
#include <ostream>

#include <stdexcept>
#include <unordered_map>

namespace dynamic {
    enum class Type {
        none=0, map, list, string, number, boolean, integer
    };

    const std::string& type_name(const Value& value);
    List_sptr create_list(std::initializer_list<Value> values={});
    Map_sptr create_map(std::initializer_list<std::pair<const std::string, Value>> entries={});
    number_t get_number(const Value& value);
    integer_t get_integer(const Value& value);
    
    inline bool is_numeric(const Value& value) {
        return std::holds_alternative<number_t>(value) ||
               std::holds_alternative<integer_t>(value);
    }

    inline number_t as_number(const Value& value) {
        if (auto num = std::get_if<number_t>(&value)) {
            return *num;
        } else if (auto num = std::get_if<integer_t>(&value)) {
            return *num;
        }
        return NAN;
    }

    class List {
    public:
        std::vector<Value> values;

        List() = default;
        List(std::vector<Value> values) : values(std::move(values)) {}

        std::string str(size_t index) const;
        number_t num(size_t index) const;
        integer_t integer(size_t index) const;
        const Map_sptr& map(size_t index) const;
        List* list(size_t index) const;
        bool flag(size_t index) const;

        inline size_t size() const {
            return values.size();
        }

        inline Value& get(size_t i) {
            return values.at(i);
        }

        List& put(std::unique_ptr<Map> value) {
            return put(Map_sptr(value.release()));
        }
        List& put(std::unique_ptr<List> value) {
            return put(List_sptr(value.release()));
        }
        List& put(const Value& value);

        Value* getValueWriteable(size_t index);

        List& putList();
        Map& putMap();

        void remove(size_t index);
    };

    class Map {
    public:
        std::unordered_map<std::string, Value> values;

        Map() = default;
        Map(std::unordered_map<std::string, Value> values) 
        : values(std::move(values)) {};

        template<typename T>
        T get(const std::string& key) const {
            if (!has(key)) {
                throw std::runtime_error("missing key '"+key+"'");
            }
            return get(key, T());
        }

        std::string get(const std::string& key, const std::string& def) const;
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

        void str(const std::string& key, std::string& dst) const;
        void num(const std::string& key, int& dst) const;
        void num(const std::string& key, float& dst) const;
        void num(const std::string& key, uint& dst) const;
        void num(const std::string& key, int64_t& dst) const;
        void num(const std::string& key, uint64_t& dst) const;
        void num(const std::string& key, ubyte& dst) const;
        void num(const std::string& key, double& dst) const;
        Map_sptr map(const std::string& key) const;
        List_sptr list(const std::string& key) const;
        void flag(const std::string& key, bool& dst) const;

        Map& put(const std::string& key, std::unique_ptr<Map> value) {
            return put(key, Map_sptr(value.release()));
        }
        Map& put(const std::string& key, std::unique_ptr<List> value) {
            return put(key, List_sptr(value.release()));
        }
        Map& put(const std::string& key, int value) {
            return put(key, Value(static_cast<integer_t>(value)));
        }
        Map& put(const std::string& key, unsigned int value) {
            return put(key, Value(static_cast<integer_t>(value)));
        }
        Map& put(const std::string& key, int64_t value) {
            return put(key, Value(static_cast<integer_t>(value)));
        }
        Map& put(const std::string& key, uint64_t value) {
            return put(key, Value(static_cast<integer_t>(value)));
        }
        Map& put(const std::string& key, float value) {
            return put(key, Value(static_cast<number_t>(value)));
        }
        Map& put(const std::string& key, double value) {
            return put(key, Value(static_cast<number_t>(value)));
        }
        Map& put(const std::string& key, bool value) {
            return put(key, Value(static_cast<bool>(value)));
        }
        Map& put(const std::string& key, const char* value) {
            return put(key, Value(value));
        }
        Map& put(const std::string& key, const Value& value);

        void remove(const std::string& key);

        List& putList(const std::string& key);
        Map& putMap(const std::string& key);

        bool has(const std::string& key) const;
        size_t size() const;
    };
}

std::ostream& operator<<(std::ostream& stream, const dynamic::Value& value);
std::ostream& operator<<(std::ostream& stream, const dynamic::Map_sptr& value);
std::ostream& operator<<(std::ostream& stream, const dynamic::List_sptr& value);

#endif // DATA_DYNAMIC_HPP_
