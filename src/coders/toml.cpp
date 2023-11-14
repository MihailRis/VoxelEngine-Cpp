#include "toml.h"
#include "commons.h"

#include <math.h>
#include <iostream>
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

const Field* Section::field(std::string name) const {
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
    for (string key : keyOrder) {
        const Section* section = sections.at(key);
        ss << "[" << key << "]\n";
        for (const string& key : section->keys()) {
            ss << key << " = ";
            const Field* field = section->field(key);
            assert(field != nullptr);
            switch (field->type) {
                case fieldtype::ftbool:
                    ss << (*((bool*)field->ptr) ? "true" : "false");
                    break;
                case fieldtype::ftint: ss << *((int*)field->ptr); break;
                case fieldtype::ftuint: ss << *((uint*)field->ptr); break;
                case fieldtype::ftfloat: ss << *((float*)field->ptr); break;
                case fieldtype::ftstring: 
                    ss << escape_string(*((const string*)field->ptr)); 
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
        pos++;
        while (hasNext()) {
            if (source[pos] == '\n') {
                pos++;
                linestart = pos;
                line++;
                break;
            }
            pos++;
        }
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

inline bool is_numeric_type(fieldtype type) {
    return type == fieldtype::ftint || type == fieldtype::ftfloat;
}

void Section::set(string name, double value) {
    const Field* field = this->field(name);
    if (field == nullptr) {
        std::cerr << "warning: unknown key '" << name << "'" << std::endl;
    } else {
        switch (field->type) {
        case fieldtype::ftbool: *(bool*)(field->ptr) = fabs(value) > 0.0; break;
        case fieldtype::ftint: *(int*)(field->ptr) = value; break;
        case fieldtype::ftuint: *(uint*)(field->ptr) = value; break;
        case fieldtype::ftfloat: *(float*)(field->ptr) = value; break;
        case fieldtype::ftstring: *(string*)(field->ptr) = std::to_string(value); break;
        default:
            std::cerr << "error: type error for key '" << name << "'" << std::endl;
        }
    }
}

void Section::set(std::string name, bool value) {
    const Field* field = this->field(name);
    if (field == nullptr) {
        std::cerr << "warning: unknown key '" << name << "'" << std::endl;
    } else {
        switch (field->type) {
        case fieldtype::ftbool: *(bool*)(field->ptr) = value; break;
        case fieldtype::ftint: *(int*)(field->ptr) = (int)value; break;
        case fieldtype::ftuint: *(uint*)(field->ptr) = (uint)value; break;
        case fieldtype::ftfloat: *(float*)(field->ptr) = (float)value; break;
        case fieldtype::ftstring: *(string*)(field->ptr) = value ? "true" : "false"; break;
        default:
            std::cerr << "error: type error for key '" << name << "'" << std::endl;
        }
    }
}

void Section::set(std::string name, std::string value) {
    const Field* field = this->field(name);
    if (field == nullptr) {
        std::cerr << "warning: unknown key '" << name << "'" << std::endl;
    } else {
        switch (field->type) {
        case fieldtype::ftstring: *(string*)(field->ptr) = value; break;
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
            string name = parseName();
            Section* section = wrapper->section(name);
            pos++;
            readSection(section);
            return;
        }
        pos--;
        string name = parseName();
        expect('=');
        c = peek();
        if (is_digit(c)) {
            double number = parseNumber(1);
            if (section) {
                section->set(name, number);
            }
        } else if (c == '-' || c == '+') {
            int sign = c == '-' ? -1 : 1;
            pos++;
            double number = parseNumber(sign);
            if (section) {
                section->set(name, number);
            }
        } else if (is_identifier_start(c)) {
            string identifier = parseName();
            if (identifier == "true" || identifier == "false") {
                bool flag = identifier == "true";
                if (section) {
                    section->set(name, flag);
                }
            }
        } else if (c == '"' || c == '\'') {
            pos++;
            string str = parseString(c);
            if (section) {
                section->set(name, str);
            }
        } else {
            throw error("feature is not supported");
        }
        expectNewLine();
    }
}