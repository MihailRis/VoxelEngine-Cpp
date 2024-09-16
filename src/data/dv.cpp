#include "dv.hpp"

#include "util/Buffer.hpp"

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

    value& value::operator=(const objects::Bytes& bytes) {
        return setBytes(std::make_shared<objects::Bytes>(bytes));
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

    void value::add(value v) {
        if (type == value_type::list) {
            return val.list->push(std::move(v));
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
        if (type == value_type::list) {
            return val.list->add(std::make_shared<objects::Object>());
        }
        throw std::runtime_error("value is not a list");
    }

    value& value::list() {
        if (type == value_type::list) {
            return val.list->add(std::make_shared<objects::List>());
        }
        throw std::runtime_error("value is not a list");
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
}

