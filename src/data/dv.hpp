#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <unordered_map>

namespace util {
    template<class T> class Buffer;
}

namespace dv {
    using integer_t = int64_t;
    using number_t = double;
    using boolean_t = bool;
    using byte_t = unsigned char;
    using key_t = std::string;

    enum class value_type : uint8_t {
        none = 0,
        object,
        list,
        bytes,
        string,
        number,
        boolean,
        integer
    };

    namespace objects {
        class Object;
        class List;
        using Bytes = util::Buffer<byte_t>;
    }

    class value;

    using list_t = std::vector<value>;
    using map_t = std::unordered_map<key_t, value>;
    using pair = std::pair<const key_t, value>;

    class value {
        value_type type = value_type::none;
        union value_u {
            integer_t integer;
            number_t number;
            boolean_t boolean;
            std::unique_ptr<std::string> string;
            std::shared_ptr<objects::Object> object;
            std::shared_ptr<objects::List> list;
            std::shared_ptr<objects::Bytes> bytes;
            value_u() {}
            ~value_u() {}
        } val;

        inline value& setBoolean(boolean_t v) {
            this->~value();
            type = value_type::boolean;
            val.boolean = v;
            return *this;
        }
        inline value& setInteger(integer_t v) {
            this->~value();
            type = value_type::integer;
            val.integer = v;
            return *this;
        }
        inline value& setNumber(number_t v) {
            this->~value();
            type = value_type::number;
            val.number = v;
            return *this;
        }
        inline value& setNone() {
            this->~value();
            type = value_type::none;
            return *this;
        }
        inline value& setString(std::string v) {
            this->~value();
            new(&val.string)std::unique_ptr<std::string>(
                std::make_unique<std::string>(std::move(v)));
            type = value_type::string;
            return *this;
        }
        inline value& setList(std::shared_ptr<objects::List> ptr) {
            this->~value();
            new(&val.list)std::shared_ptr<objects::List>(std::move(ptr));
            type = value_type::list;
            return *this;
        }
        inline value& setObject(std::shared_ptr<objects::Object> ptr) {
            this->~value();
            new(&val.object)std::shared_ptr<objects::Object>(std::move(ptr));
            type = value_type::object;
            return *this;
        }
        inline value& setBytes(std::shared_ptr<objects::Bytes> ptr) {
            this->~value();
            new(&val.bytes)std::shared_ptr<objects::Bytes>(std::move(ptr));
            type = value_type::bytes;
            return *this;
        }
    public:
        value() : type(value_type::none) {}
        value(value_type type);
        
        template<class T>
        value(T v) {
            this->operator=(v);
        }

        value(const value& v) noexcept : type(value_type::none) {
            this->operator=(v);
        }

        value(value&& v) noexcept : type(v.type) {
            switch (v.type) {
                case value_type::none:
                    break;
                case value_type::integer:
                    val.integer = v.val.integer;
                    break;
                case value_type::number:
                    val.number = v.val.number;
                    break;
                case value_type::boolean:
                    val.boolean = v.val.boolean;
                    break;
                case value_type::string:
                    new(&val.string)std::unique_ptr<std::string>(
                        std::move(v.val.string));
                    break;
                case value_type::object:
                    new(&val.object)std::shared_ptr<objects::Object>(
                        std::move(v.val.object));
                    break;
                case value_type::list:
                    new(&val.list)std::shared_ptr<objects::List>(
                        std::move(v.val.list));
                    break;
                case value_type::bytes:
                    new(&val.list)std::shared_ptr<objects::Bytes>(
                        std::move(v.val.bytes));
                    break;
            }
        }

        ~value() {
            switch (type) {
                case value_type::object:
                    val.object.reset();
                    break;
                case value_type::list:
                    val.list.reset();
                    break;
                case value_type::bytes:
                    val.bytes.reset();
                    break;
                case value_type::string:
                    val.string.reset();
                    break;
                default:
                    break;
            }
        }

        inline value& operator=(int8_t v) {
            return setInteger(v);
        }
        inline value& operator=(int16_t v) {
            return setInteger(v);
        }
        inline value& operator=(int32_t v) {
            return setInteger(v);
        }
        inline value& operator=(int64_t v) {
            return setInteger(v);
        }
        inline value& operator=(uint8_t v) {
            return setInteger(v);
        }
        inline value& operator=(uint16_t v) {
            return setInteger(v);
        }
        inline value& operator=(uint32_t v) {
            return setInteger(v);
        }
        inline value& operator=(uint64_t v) {
            return setInteger(v);
        }
        inline value& operator=(float v) {
            return setNumber(v);
        }
        inline value& operator=(double v) {
            return setNumber(v);
        }
        inline value& operator=(bool v) {
            return setBoolean(v);
        }
        inline value& operator=(std::string_view v) {
            return setString(std::string(v));
        }
        inline value& operator=(std::string v) {
            return setString(std::move(v));
        }
        inline value& operator=(const char* v) {
            return setString(v);
        }
        inline value& operator=(std::shared_ptr<objects::List> ptr) {
            return setList(std::move(ptr));
        }
        inline value& operator=(std::shared_ptr<objects::Object> ptr) {
            return setObject(std::move(ptr));
        }
        inline value& operator=(std::shared_ptr<objects::Bytes> ptr) {
            return setBytes(std::move(ptr));
        }
        value& operator=(const objects::Bytes& bytes);

        inline value& operator=(const value& v) {
            switch (v.type) {
                case value_type::object:
                    setObject(v.val.object);
                    break;
                case value_type::list:
                    setList(v.val.list);
                    break;
                case value_type::bytes:
                    setBytes(v.val.bytes);
                    break;
                case value_type::string:
                    setString(*v.val.string);
                    break;
                case value_type::boolean:
                    setBoolean(v.val.boolean);
                    break;
                case value_type::integer:
                    setInteger(v.val.integer);
                    break;
                case value_type::number:
                    setNumber(v.val.number);
                    break;
                case value_type::none:
                    setNone();
                    break;
            }
            return *this;
        }

        void add(value v);

        template<class T>
        inline void add(T v) {
            return add(value(v));
        }

        value& operator[](const key_t& key);

        const value& operator[](const key_t& key) const;

        value& operator[](size_t index);

        const value& operator[](size_t index) const;

        value& object(const key_t& key);
        
        value& list(const key_t& key);

        value& object();
        
        value& list();

        list_t::iterator begin();
        list_t::iterator end();

        list_t::const_iterator begin() const;
        list_t::const_iterator end() const;

        const std::string& asString() const;

        integer_t asInteger() const;

        number_t asNumber() const;

        boolean_t asBoolean() const;

        objects::Bytes& asBytes();

        const objects::Bytes& asBytes() const;   

        const objects::Object& asObject() const;

        inline value_type getType() const {
            return type;
        }

        const size_t size() const;

        const size_t length() const {
            return size();
        }
        inline bool empty() const {
            return size() == 0;
        }
    };

    inline value none = value();

    using reference = value&;
    using const_reference = const value&;
}

namespace dv::objects {
    class Object {
        map_t map;
    public:
        Object() = default;
        Object(std::initializer_list<pair> pairs) : map(pairs) {}
        Object(const Object&) = delete;
        ~Object() = default;

        reference operator[](const key_t& key) {
            return map[key];
        }
        const_reference operator[](const key_t& key) const {
            return map.at(key);
        }

        map_t::const_iterator begin() const {
            return map.begin();
        }
        map_t::const_iterator end() const {
            return map.end();
        }

        const size_t size() const {
            return map.size();
        }
    };

    class List {
        list_t list;
    public:
        List() = default;
        List(std::initializer_list<value> values) : list(values) {}
        List(const List&) = delete;
        ~List() = default;

        reference operator[](std::size_t index) {
            return list.at(index);
        }

        const_reference operator[](std::size_t index) const {
            return list.at(index);
        }

        void push(value v) {
            list.push_back(std::move(v));
        }

        reference add(value v) {
            list.push_back(std::move(v));
            return list[list.size()-1];
        }

        auto begin() {
            return list.begin();
        }
        auto end() {
            return list.end();
        }

        list_t::const_iterator begin() const {
            return list.begin();
        }
        list_t::const_iterator end() const {
            return list.end();
        }

        const size_t size() const {
            return list.size();
        }
    };
}

namespace dv {
    inline value object() {
        return std::make_shared<objects::Object>();
    }

    inline value list() {
        return std::make_shared<objects::List>();
    }

    inline value list(std::initializer_list<value> values) {
        return std::make_shared<objects::List>(values);
    }
}
