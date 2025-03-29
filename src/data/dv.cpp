#include "dv.hpp"

#include <iostream>

#include "util/Buffer.hpp"

namespace dv {
    value& value::operator[](const key_t& key) {
        check_type(type, value_type::object);
        return (*val.object)[key];
    }
    const value& value::operator[](const key_t& key) const {
        check_type(type, value_type::object);
        return (*val.object)[key];
    }

    static void apply_method(value& dst, value&& val, std::string_view method, bool deep) {
        if (!val.isList()) {
            return;
        }
        if (dst == nullptr) {
            dst = dv::list();
        } else if (!dst.isList()) {
            return;
        }
        if (method == "append") {
            if (dst == nullptr) {
                dst = std::forward<value>(val);
                return;
            }
            for (auto& elem : val) {
                dst.add(std::move(elem));
            }
        }
    }

    static void merge_elem(value& self, const key_t& key, value&& val, bool deep) {
        auto& dst = self[key];
        size_t pos = key.rfind('@');
        if (pos != std::string::npos) {
            auto method = std::string_view(key).substr(pos + 1);
            auto& field = self[key.substr(0, pos)];
            apply_method(field, std::forward<value>(val), method, deep);
            return;
        }
        if (val.isObject() && dst == nullptr) {
            dst = dv::object();
        }
        if (dst.isObject() && deep) {
            dst.merge(std::forward<dv::value>(val), true);
        } else {
            dst = std::forward<dv::value>(val);
        }
    }

    void value::merge(dv::value&& other, bool deep) {
        check_type(other.type, value_type::object);
        for (auto& [key, val] : *other.val.object) {
            merge_elem(*this, key, std::forward<value>(val), deep);
        }
    }

    value& value::operator=(const objects::Bytes& bytes) {
        return setBytes(std::make_shared<objects::Bytes>(bytes));
    }

    value& value::operator[](size_t index) {
        check_type(type, value_type::list);
        return (*val.list)[index];
    }
    const value& value::operator[](size_t index) const {
        check_type(type, value_type::list);
        return (*val.list)[index];
    }

    void value::add(value v) {
        check_type(type, value_type::list);
        return val.list->push_back(std::move(v));
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
        check_type(type, value_type::list);
        val.list->push_back(std::make_shared<objects::Object>());
        return val.list->operator[](val.list->size()-1);
    }

    value& value::list() {
        check_type(type, value_type::list);
        val.list->push_back(std::make_shared<objects::List>());
        return val.list->operator[](val.list->size()-1);
    }

    list_t::iterator value::begin() {
        check_type(type, value_type::list);
        return val.list->begin();
    }

    list_t::iterator value::end() {
        check_type(type, value_type::list);
        return val.list->end();
    }

    list_t::const_iterator value::begin() const {
        check_type(type, value_type::list);
        const auto& constlist = *val.list;
        return constlist.begin();
    }

    list_t::const_iterator value::end() const {
        check_type(type, value_type::list);
        const auto& constlist = *val.list;
        return constlist.end();
    }

    const std::string& value::asString() const {
        check_type(type, value_type::string);
        return *val.string;
    }

    integer_t value::asInteger() const {
        if (type == value_type::integer) {
            return val.integer;
        } else if (type == value_type::number) {
            return static_cast<integer_t>(val.number);
        }
        throw_type_error(type, value_type::integer);
        return 0; // unreachable
    }

    number_t value::asNumber() const {
        if (type == value_type::number) {
            return val.number;
        } else if (type == value_type::integer) {
            return static_cast<number_t>(val.integer);
        }
        throw_type_error(type, value_type::integer);
        return 0; // unreachable
    }

    boolean_t value::asBoolean() const {
        if (type == value_type::none) {
            return false;
        }
        check_type(type, value_type::boolean);
        return val.boolean;
    }

    objects::Bytes& value::asBytes() {
        check_type(type, value_type::bytes);
        return *val.bytes;
    }

    const objects::Bytes& value::asBytes() const {
        check_type(type, value_type::bytes);
        return *val.bytes;
    }

    const objects::Object& value::asObject() const {
        check_type(type, value_type::object);
        return *val.object;
    }

    size_t value::size() const noexcept {
        switch (type) {
            case value_type::list:
                return val.list->size();
            case value_type::object:
                return val.object->size();
            case value_type::string:
                return val.string->size();
            default:
                return 0;
        }
    }

    bool value::has(const key_t& k) const {
        if (type == value_type::object) {
            return val.object->find(k) != val.object->end();
        }
        return false;
    }

    void value::erase(const key_t& key) {
        check_type(type, value_type::object);
        val.object->erase(key);
    }

    void value::erase(size_t index) {
        check_type(type, value_type::list);
        val.list->erase(val.list->begin() + index);
    }
}

#include "coders/json.hpp"

std::ostream& operator<<(std::ostream& stream, const dv::value& value) {
    return stream << json::stringify(value, false);
}
