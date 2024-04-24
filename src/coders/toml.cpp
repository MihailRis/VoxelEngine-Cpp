#include "toml.h"
#include "commons.h"
#include "../util/stringutil.h"

#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <assert.h>

// FIXME: refactor this monster

using namespace toml;

Section::Section(std::string name) : name(name) {
}

void Section::add(std::string name, Field field) {
    if (fields.find(name) != fields.end()) {
        throw std::runtime_error("field duplication");
    }
    fields[name] = field;
    keyOrder.push_back(name);
}

void Section::add(std::string name, bool* ptr) {
    add(name, {fieldtype::ftbool, ptr});
}

void Section::add(std::string name, int* ptr) {
    add(name, {fieldtype::ftint, ptr});
}

void Section::add(std::string name, uint* ptr) {
    add(name, {fieldtype::ftuint, ptr});
}

void Section::add(std::string name, int64_t* ptr) {
    add(name, {fieldtype::ftint64, ptr});
}

void Section::add(std::string name, float* ptr) {
    add(name, {fieldtype::ftfloat, ptr});
}

void Section::add(std::string name, double* ptr) {
    add(name, {fieldtype::ftdouble, ptr});
}

void Section::add(std::string name, std::string* ptr) {
    add(name, {fieldtype::ftstring, ptr});
}

const std::string& Section::getName() const {
    return name;
}

const Field* Section::field(const std::string& name) const {
    auto found = fields.find(name);
    if (found == fields.end()) {
        return nullptr;
    }
    return &found->second;
}

const std::vector<std::string>& Section::keys() const {
    return keyOrder;
}

Wrapper::~Wrapper() {
    for (auto entry : sections) {
        delete entry.second;
    }
}

Section& Wrapper::add(std::string name) {
    if (sections.find(name) != sections.end()) {
        throw std::runtime_error("section duplication");
    }
    Section* section = new Section(name);
    sections[name] = section;
    keyOrder.push_back(name);
    return *section;
}

Section* Wrapper::section(std::string name) {
    auto found = sections.find(name);
    if (found == sections.end()) {
        return nullptr;
    }
    return found->second;
}

std::string Wrapper::write() const {
    std::stringstream ss;
    for (const std::string& key : keyOrder) {
        const Section* section = sections.at(key);
        ss << "[" << key << "]\n";
        for (const std::string& key : section->keys()) {
            ss << key << " = ";
            const Field* field = section->field(key);
            assert(field != nullptr);
            switch (field->type) {
                case fieldtype::ftbool:
                    ss << (*((bool*)field->ptr) ? "true" : "false");
                    break;
                case fieldtype::ftint: ss << *((int*)field->ptr); break;
                case fieldtype::ftuint: ss << *((uint*)field->ptr); break;
                case fieldtype::ftint64: ss << *((int64_t*)field->ptr); break;
                case fieldtype::ftfloat: ss << *((float*)field->ptr); break;
                case fieldtype::ftdouble: ss << *((double*)field->ptr); break;
                case fieldtype::ftstring: 
                    ss << util::escape(*((const std::string*)field->ptr)); 
                    break;
            }
            ss << "\n";
        }
        ss << "\n";
    }
    return ss.str();
}

Reader::Reader(Wrapper* wrapper, std::string file, std::string source) 
: BasicParser(file, source), wrapper(wrapper) {
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
    readSection(nullptr);
}

void Section::set(const std::string& name, double value) {
    const Field* field = this->field(name);
    if (field == nullptr) {
        std::cerr << "warning: unknown key '" << name << "'" << std::endl;
    } else {
        switch (field->type) {
        case fieldtype::ftbool: *(bool*)(field->ptr) = fabs(value) > 0.0; break;
        case fieldtype::ftint: *(int*)(field->ptr) = value; break;
        case fieldtype::ftuint: *(uint*)(field->ptr) = value; break;
        case fieldtype::ftint64: *(int64_t*)(field->ptr) = value; break;
        case fieldtype::ftfloat: *(float*)(field->ptr) = value; break;
        case fieldtype::ftdouble: *(double*)(field->ptr) = value; break;
        case fieldtype::ftstring: *(std::string*)(field->ptr) = std::to_string(value); break;
        default:
            std::cerr << "error: type error for key '" << name << "'" << std::endl;
        }
    }
}

void Section::set(const std::string& name, bool value) {
    const Field* field = this->field(name);
    if (field == nullptr) {
        std::cerr << "warning: unknown key '" << name << "'" << std::endl;
    } else {
        switch (field->type) {
        case fieldtype::ftbool: *(bool*)(field->ptr) = value; break;
        case fieldtype::ftint: *(int*)(field->ptr) = (int)value; break;
        case fieldtype::ftuint: *(uint*)(field->ptr) = (uint)value; break;
        case fieldtype::ftint64: *(int64_t*)(field->ptr) = (int64_t)value; break;
        case fieldtype::ftfloat: *(float*)(field->ptr) = (float)value; break;
        case fieldtype::ftdouble: *(double*)(field->ptr) = (double)value; break;
        case fieldtype::ftstring: *(std::string*)(field->ptr) = value ? "true" : "false"; break;
        default:
            std::cerr << "error: type error for key '" << name << "'" << std::endl;
        }
    }
}

void Section::set(const std::string& name, std::string value) {
    const Field* field = this->field(name);
    if (field == nullptr) {
        std::cerr << "warning: unknown key '" << name << "'" << std::endl;
    } else {
        switch (field->type) {
        case fieldtype::ftstring: *(std::string*)(field->ptr) = value; break;
        default:
            std::cerr << "error: type error for key '" << name << "'" << std::endl;
        }
    }
}

void Reader::readSection(Section* section /*nullable*/) {
    while (hasNext()) {
        skipWhitespace();
        if (!hasNext()) {
            break;
        }
        char c = nextChar();
        if (c == '[') {
            std::string name = parseName();
            Section* section = wrapper->section(name);
            pos++;
            readSection(section);
            return;
        }
        pos--;
        std::string name = parseName();
        expect('=');
        c = peek();
        if (is_digit(c)) {
            number_u num;
            if (parseNumber(1, num)) {
                if (section)
                    section->set(name, (double)std::get<integer_t>(num));
            } else {
                if (section)
                    section->set(name, std::get<number_t>(num));
            }
        } else if (c == '-' || c == '+') {
            int sign = c == '-' ? -1 : 1;
            pos++;
            number_u num;
            if (parseNumber(sign, num)) {
                if (section)
                    section->set(name, (double)std::get<integer_t>(num));
            } else {
                if (section)
                    section->set(name, std::get<number_t>(num));
            }
        } else if (is_identifier_start(c)) {
            std::string identifier = parseName();
            if (identifier == "true" || identifier == "false") {
                bool flag = identifier == "true";
                if (section) {
                    section->set(name, flag);
                }
            } else if (identifier == "inf") {
                if (section) {
                    section->set(name, INFINITY);
                }
            } else if (identifier == "nan") {
                if (section) {
                    section->set(name, NAN);
                }
            }
        } else if (c == '"' || c == '\'') {
            pos++;
            std::string str = parseString(c);
            if (section) {
                section->set(name, str);
            }
        } else {
            throw error("feature is not supported");
        }
        expectNewLine();
    }
}
