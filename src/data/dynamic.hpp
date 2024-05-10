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

    enum class Type {
        none=0, map, list, string, number, boolean, integer
    };

    using Map_sptr = std::shared_ptr<Map>;
    using List_sptr = std::shared_ptr<List>;

    struct none {};

    inline constexpr none NONE = {};

    using Value = std::variant<
        none,
        Map_sptr,
        List_sptr,
        std::string,
        number_t,
        bool,
        integer_t
    >;

    const std::string& type_name(const Value& value);
    List_sptr create_list(std::initializer_list<Value> values={});
    Map_sptr create_map(std::initializer_list<std::pair<const std::string, Value>> entries={});
    number_t get_number(const Value& value);
    integer_t get_integer(const Value& value);
    
    inline bool is_numeric(const Value& value) {
        return std::holds_alternative<number_t>(value) ||
               std::holds_alternative<integer_t>(value);
    }

    class List {
    public:
        std::vector<Value> values;

        List() {}
        List(std::vector<Value> values) : values(std::move(values)) {}

        std::string str(size_t index) const;
        number_t num(size_t index) const;
        integer_t integer(size_t index) const;
        Map* map(size_t index) const;
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

        Map() {}
        Map(std::unordered_map<std::string, Value> values) 
        : values(std::move(values)) {};

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

        void str(const std::string& key, std::string& dst) const;
        void num(const std::string& key, int& dst) const;
        void num(const std::string& key, float& dst) const;
        void num(const std::string& key, uint& dst) const;
        void num(const std::string& key, int64_t& dst) const;
        void num(const std::string& key, uint64_t& dst) const;
        void num(const std::string& key, ubyte& dst) const;
        void num(const std::string& key, double& dst) const;
        Map* map(const std::string& key) const;
        List* list(const std::string& key) const;
        void flag(const std::string& key, bool& dst) const;

        Map& put(std::string key, std::unique_ptr<Map> value) {
            return put(key, Map_sptr(value.release()));
        }
        Map& put(std::string key, std::unique_ptr<List> value) {
            return put(key, List_sptr(value.release()));
        }
        Map& put(std::string key, const Value& value);

        void remove(const std::string& key);

        List& putList(std::string key);
        Map& putMap(std::string key);

        bool has(const std::string& key) const;
        size_t size() const;
    };
}

std::ostream& operator<<(std::ostream& stream, const dynamic::Value& value);
std::ostream& operator<<(std::ostream& stream, const dynamic::Map_sptr& value);
std::ostream& operator<<(std::ostream& stream, const dynamic::List_sptr& value);

#endif // DATA_DYNAMIC_HPP_
