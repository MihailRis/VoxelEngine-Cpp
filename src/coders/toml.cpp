#include "toml.h"
#include "commons.h"

#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <assert.h>

using std::string;

using namespace toml;

Section::Section(string name) : name(name) {
}

void Section::add(std::string name, Field field) {
    if (fields.find(name) != fields.end()) {
        throw std::runtime_error("field duplication");
    }
    fields[name] = field;
    keyOrder.push_back(name);
}

void Section::add(string name, bool* ptr) {
    add(name, {fieldtype::ftbool, ptr});
}

void Section::add(string name, int* ptr) {
    add(name, {fieldtype::ftint, ptr});
}

void Section::add(string name, uint* ptr) {
    add(name, {fieldtype::ftuint, ptr});
}

void Section::add(string name, float* ptr) {
    add(name, {fieldtype::ftfloat, ptr});
}

void Section::add(string name, string* ptr) {
    add(name, {fieldtype::ftstring, ptr});
}

string Section::getName() const {
    return name;
}

std::optional<std::reference_wrapper<const Field>> Section::field(std::string name) const {
    auto found = fields.find(name);
    if (found == fields.end()) {
        return std::nullopt;
    }
    return found->second;
}

const std::vector<std::string>& Section::keys() const {
    return keyOrder;
}

Section& Wrapper::add(std::string name) {
    if (sections.find(name) != sections.end()) {
        throw std::runtime_error("section duplication");
    }
    Section& section = sections.emplace(name, name).first->second;
    keyOrder.push_back(name);
    return section;
}

std::optional<std::reference_wrapper<Section>> Wrapper::section(std::string name) {
    auto found = sections.find(name);
    if (found == sections.end()) {
        return std::nullopt;
    }
    return found->second;
}

std::string Wrapper::write() const {
    std::stringstream ss;
    for (string key : keyOrder) {
        const Section& section = sections.at(key);
        ss << "[" << key << "]\n";
        for (const string& key : section.keys()) {
            ss << key << " = ";
            assert(section.field(key).has_value());
            const Field& field = section.field(key).value();
            switch (field.type) {
                case fieldtype::ftbool:
                    ss << (*((bool*)field.ptr) ? "true" : "false");
                    break;
                case fieldtype::ftint: ss << *((int*)field.ptr); break;
                case fieldtype::ftuint: ss << *((uint*)field.ptr); break;
                case fieldtype::ftfloat: ss << *((float*)field.ptr); break;
                case fieldtype::ftstring:
                    ss << escape_string(*((const string*)field.ptr));
                    break;
            }
            ss << "\n";
        }
        ss << "\n";
    }
    return ss.str();
}

Reader::Reader(Wrapper* wrapper, string file, string source) : BasicParser(file, source), wrapper(wrapper) {
}

void Reader::skipWhitespace() {
    BasicParser::skipWhitespace();
    if (hasNext() && source[pos] == '#') {
        skipLine();
        if (hasNext() && is_whitespace(peek())) {
            skipWhitespace();
        }
    }
}

void Reader::read() {
    skipWhitespace();
    if (!hasNext()) {
        return;
    }
    readSection(std::nullopt);
}

inline bool is_numeric_type(fieldtype type) {
    return type == fieldtype::ftint || type == fieldtype::ftfloat;
}

void Section::set(string name, double value) {
    auto fieldOpt = this->field(name);
    if (!fieldOpt.has_value()) {
        std::cerr << "warning: unknown key '" << name << "'" << std::endl;
    } else {
        const Field& field = fieldOpt.value();
        switch (field.type) {
        case fieldtype::ftbool: *(bool*)(field.ptr) = fabs(value) > 0.0; break;
        case fieldtype::ftint: *(int*)(field.ptr) = value; break;
        case fieldtype::ftuint: *(uint*)(field.ptr) = value; break;
        case fieldtype::ftfloat: *(float*)(field.ptr) = value; break;
        case fieldtype::ftstring: *(string*)(field.ptr) = std::to_string(value); break;
        default:
            std::cerr << "error: type error for key '" << name << "'" << std::endl;
        }
    }
}

void Section::set(std::string name, bool value) {
    auto fieldOpt = this->field(name);
    if (!fieldOpt.has_value()) {
        std::cerr << "warning: unknown key '" << name << "'" << std::endl;
    } else {
        const Field& field = fieldOpt.value();
        switch (field.type) {
        case fieldtype::ftbool: *(bool*)(field.ptr) = value; break;
        case fieldtype::ftint: *(int*)(field.ptr) = (int)value; break;
        case fieldtype::ftuint: *(uint*)(field.ptr) = (uint)value; break;
        case fieldtype::ftfloat: *(float*)(field.ptr) = (float)value; break;
        case fieldtype::ftstring: *(string*)(field.ptr) = value ? "true" : "false"; break;
        default:
            std::cerr << "error: type error for key '" << name << "'" << std::endl;
        }
    }
}

void Section::set(std::string name, std::string value) {
    auto fieldOpt = this->field(name);
    if (!fieldOpt.has_value()) {
        std::cerr << "warning: unknown key '" << name << "'" << std::endl;
    } else {
        const Field& field = fieldOpt.value();
        switch (field.type) {
        case fieldtype::ftstring: *(string*)(field.ptr) = value; break;
        default:
            std::cerr << "error: type error for key '" << name << "'" << std::endl;
        }
    }
}

void Reader::readSection(std::optional<std::reference_wrapper<Section>> section) {
    while (hasNext()) {
        skipWhitespace();
        if (!hasNext()) {
            break;
        }
        char c = nextChar();
        if (c == '[') {
            string name = parseName();
            auto section = wrapper->section(name);
            pos++;
            readSection(section);
            return;
        }
        pos--;
        string name = parseName();
        expect('=');
        c = peek();
        if (is_digit(c)) {
            number_u num;
            if (parseNumber(1, num)) {
                if (section.has_value())
                    section.value().get().set(name, (double)num.ival);
            } else {
                if (section.has_value())
                    section.value().get().set(name, num.fval);
            }
        } else if (c == '-' || c == '+') {
            int sign = c == '-' ? -1 : 1;
            pos++;
            number_u num;
            if (parseNumber(sign, num)) {
                if (section.has_value())
                    section.value().get().set(name, (double)num.ival);
            } else {
                if (section.has_value())
                    section.value().get().set(name, num.fval);
            }
        } else if (is_identifier_start(c)) {
            string identifier = parseName();
            if (identifier == "true" || identifier == "false") {
                bool flag = identifier == "true";
                if (section.has_value()) {
                    section.value().get().set(name, flag);
                }
            } else if (identifier == "inf") {
                if (section.has_value()) {
                    section.value().get().set(name, INFINITY);
                }
            } else if (identifier == "nan") {
                if (section.has_value()) {
                    section.value().get().set(name, NAN);
                }
            }
        } else if (c == '"' || c == '\'') {
            pos++;
            string str = parseString(c);
            if (section.has_value()) {
                section.value().get().set(name, str);
            }
        } else {
            throw error("feature is not supported");
        }
        expectNewLine();
    }
}
