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

        value& setBoolean(boolean_t v) {
            this->~value();
            type = value_type::boolean;
            val.boolean = v;
            return *this;
        }
        value& setInteger(integer_t v) {
            this->~value();
            type = value_type::integer;
            val.integer = v;
            return *this;
        }
        value& setNumber(number_t v) {
            this->~value();
            type = value_type::number;
            val.number = v;
            return *this;
        }
        value& setNone() {
            this->~value();
            type = value_type::none;
            return *this;
        }
        value& setString(std::string v) {
            this->~value();
            new(&val.string)std::unique_ptr<std::string>();
            val.string = std::make_unique<std::string>(v);
            type = value_type::string;
            return *this;
        }
        value& setList(std::shared_ptr<objects::List> ptr) {
            this->~value();
            new(&val.list)std::shared_ptr<objects::List>();
            type = value_type::list;
            val.list = ptr;
            return *this;
        }
        value& setObject(std::shared_ptr<objects::Object> ptr) {
            this->~value();
            new(&val.object)std::shared_ptr<objects::Object>();
            type = value_type::object;
            val.object = ptr;
            return *this;
        }
        value& setBytes(std::shared_ptr<objects::Bytes> ptr) {
            this->~value();
            new(&val.bytes)std::shared_ptr<objects::Bytes>();
            type = value_type::bytes;
            val.bytes = ptr;
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

        value(value&& v) noexcept {
            switch (v.type) {
                case value_type::none:
                    setNone();
                    break;
                case value_type::integer:
                    setInteger(v.val.integer);
                    break;
                case value_type::number:
                    setNumber(v.val.number);
                    break;
                case value_type::boolean:
                    setBoolean(v.val.boolean);
                    break;
                case value_type::string:
                    setString(*v.val.string);
                    break;
                case value_type::object:
                    setObject(v.val.object);
                    break;
                case value_type::list:
                    setList(v.val.list);
                    break;
                case value_type::bytes:
                    setBytes(v.val.bytes);
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

        value& operator=(int8_t v) {
            return setInteger(v);
        }
        value& operator=(int16_t v) {
            return setInteger(v);
        }
        value& operator=(int32_t v) {
            return setInteger(v);
        }
        value& operator=(int64_t v) {
            return setInteger(v);
        }
        value& operator=(uint8_t v) {
            return setInteger(v);
        }
        value& operator=(uint16_t v) {
            return setInteger(v);
        }
        value& operator=(uint32_t v) {
            return setInteger(v);
        }
        value& operator=(uint64_t v) {
            return setInteger(v);
        }
        value& operator=(float v) {
            return setNumber(v);
        }
        value& operator=(double v) {
            return setNumber(v);
        }
        value& operator=(bool v) {
            return setBoolean(v);
        }
        value& operator=(std::string_view v) {
            return setString(std::string(v));
        }
        value& operator=(std::string v) {
            return setString(std::move(v));
        }
        value& operator=(const char* v) {
            return setString(v);
        }
        value& operator=(std::shared_ptr<objects::List> ptr) {
            return setList(ptr);
        }
        value& operator=(std::shared_ptr<objects::Object> ptr) {
            return setObject(ptr);
        }
        value& operator=(std::shared_ptr<objects::Bytes> ptr) {
            return setBytes(ptr);
        }
        value& operator=(const value& v) {
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

        value& add(value v);

        template<class T>
        value& add(T v) {
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

        const size_t size() const;

        const size_t length() const {
            return size();
        }
    };

    using reference = value&;
    using const_reference = const value&;

    value list();
    value object();

    value list(std::initializer_list<value> values);
}

#include "util/Buffer.hpp"

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
    value::value(value_type type) : type(type) {
        switch (type) {
            case value_type::object:
                val.object = std::make_shared<objects::Object>();
                break;
            case value_type::list:
                val.list = std::make_shared<objects::List>();
                break;
            case value_type::bytes:
                val.bytes = nullptr; // no default size
                break;
            case value_type::string:
                val.string = std::make_unique<std::string>("");
                break;
            default:
                break;
        }
    }

    value& value::operator[](const key_t& key) {
        if (type == value_type::object) {
            return (*val.object)[key];
        }
        throw std::runtime_error("value is not an object");
    }
    const value& value::operator[](const key_t& key) const {
        if (type == value_type::object) {
            return (*val.object)[key];
        }
        throw std::runtime_error("value is not an object");
    }

    value& value::operator[](size_t index) {
        if (type == value_type::list) {
            return (*val.list)[index];
        }
        throw std::runtime_error("value is not a list");
    }
    const value& value::operator[](size_t index) const {
        if (type == value_type::list) {
            return (*val.list)[index];
        }
        throw std::runtime_error("value is not a list");
    }

    value& value::add(value v) {
        if (type == value_type::list) {
            return val.list->add(std::move(v));
        }
        throw std::runtime_error("value is not a list");
    }

    value& value::object(const key_t& key) {
        reference ref = this->operator[](key);
        ref = dv::object();
        return ref;
    }

    value& value::list(const key_t& key) {
        reference ref = this->operator[](key);
        ref = dv::list();
        return ref;
    }

    value& value::object() {
        return add(dv::object());
    }

    value& value::list() {
        return add(dv::list());
    }

    list_t::iterator value::begin() {
        if (type == value_type::list) {
            return val.list->begin();
        }
        throw std::runtime_error("value is not a list");
    }

    list_t::iterator value::end() {
        if (type == value_type::list) {
            return val.list->end();
        }
        throw std::runtime_error("value is not a list");
    }

    list_t::const_iterator value::begin() const {
        if (type == value_type::list) {
            const auto& constlist = *val.list;
            return constlist.begin();
        }
        throw std::runtime_error("value is not a list");
    }

    list_t::const_iterator value::end() const {
        if (type == value_type::list) {
            const auto& constlist = *val.list;
            return constlist.end();
        }
        throw std::runtime_error("value is not a list");
    }

    const std::string& value::asString() const {
        if (type == value_type::string) {
            return *val.string;
        }
        throw std::runtime_error("type error");
    }

    integer_t value::asInteger() const {
        if (type == value_type::integer) {
            return val.integer;
        } else if (type == value_type::number) {
            return static_cast<integer_t>(val.number);
        }
        throw std::runtime_error("type error");
    }

    number_t value::asNumber() const {
        if (type == value_type::number) {
            return val.number;
        } else if (type == value_type::integer) {
            return static_cast<number_t>(val.integer);
        }
        throw std::runtime_error("type error");
    }

    boolean_t value::asBoolean() const {
        if (type == value_type::boolean) {
            return val.boolean;
        } else if (type == value_type::integer) {
            return val.integer != 0;
        }
        throw std::runtime_error("type error");
    }

    objects::Bytes& value::asBytes() {
        if (type == value_type::bytes) {
            return *val.bytes;
        }
        throw std::runtime_error("type error");
    }

    const objects::Bytes& value::asBytes() const {
        if (type == value_type::bytes) {
            return *val.bytes;
        }
        throw std::runtime_error("type error");
    }

    const objects::Object& value::asObject() const {
        if (type == value_type::object) {
            return *val.object;
        }
        throw std::runtime_error("type error");
    }

    const size_t value::size() const {
        switch (type) {
            case value_type::list:
                return val.list->size();
            case value_type::object:
                return val.object->size();
            case value_type::string:
                return val.string->size();
            default:
                throw std::runtime_error("type error");
        }
    }

    value object() {
        return std::make_shared<objects::Object>();
    }

    value list() {
        return std::make_shared<objects::List>();
    }

    value list(std::initializer_list<value> values) {
        return std::make_shared<objects::List>(values);
    }
}
