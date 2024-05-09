#include "CommandsInterpreter.hpp"

#include "../coders/commons.hpp"
#include "../util/stringutil.hpp"

#include <iostream>

using namespace cmd;

inline bool is_cmd_identifier_part(char c) {
    return is_identifier_part(c) || c == '.' || c == '$' || c == '@';
}

inline bool is_cmd_identifier_start(char c) {
    return is_identifier_start(c) || c == '.' || c == '$' || c == '@';
}

class CommandParser : BasicParser {
    std::string parseIdentifier() {
        char c = peek();
        if (!is_identifier_start(c) && c != '$') {
            if (c == '"') {
                pos++;
                return parseString(c);
            }
            throw error("identifier expected");
        }
        int start = pos;
        while (hasNext() && is_cmd_identifier_part(source[pos])) {
            pos++;
        }
        return std::string(source.substr(start, pos-start));
    }

    std::unordered_map<std::string, ArgType> types {
        {"num", ArgType::number},
        {"int", ArgType::integer},
        {"str", ArgType::string},
        {"@", ArgType::selector},
        {"enum", ArgType::enumvalue},
    };
public:
    CommandParser(std::string_view filename, std::string_view source) 
    : BasicParser(filename, source) {
    }

    ArgType parseType() {
        if (peek() == '[') {
            std::cout << "type.name: enum" << std::endl;
            return ArgType::enumvalue;
        }
        std::string name = parseIdentifier();
        std::cout << "type.name: " << name << std::endl;
        auto found = types.find(name);
        if (found != types.end()) {
            return found->second;
        } else {
            throw error("unknown type "+util::quote(name));
        }
    }

    dynamic::Value parseValue() {
        char c = peek();
        if (is_cmd_identifier_start(c)) {
            auto str = parseIdentifier();
            if (str == "true") {
                return true;
            } else if (str == "false") {
                return false;
            } else if (str == "none" || str == "nil" || str == "null") {
                return dynamic::NONE;
            }
            return str;
        }
        if (c == '"') {
            return parseIdentifier();
        }
        if (c == '+' || c == '-' || is_digit(c)) {
            return parseNumber(c == '-' ? -1 : 1);
        }
        throw error("invalid character '"+std::string({c})+"'");
    }

    std::string parseEnum() {
        if (peek() == '[') {
            nextChar();
            if (peek() == ']') {
                throw error("empty enumeration is not allowed");
            }
            auto enumvalue = std::string(readUntil(']'));
            size_t offset = enumvalue.find(' ');
            if (offset != std::string::npos) {
                goBack(enumvalue.length()-offset);
                throw error("use '|' as separator, not a space");
            }
            nextChar();
            return enumvalue;
        } else {
            expect('$');
            goBack();
            return parseIdentifier();
        }
    }

    Argument parseArgument() {
        std::string name = parseIdentifier();
        expect(':');
        std::cout << "arg.name: " << name << std::endl;
        ArgType type = parseType();
        std::string enumname = "";
        if (type == ArgType::enumvalue) {
            enumname = parseEnum();
            std::cout << "enum: " << enumname << std::endl;
        }
        bool optional = false;
        dynamic::Value def {};
        dynamic::Value origin {};
        bool loop = true;
        while (hasNext() && loop) {
            char c = peek();
            switch (c) {
                case '=':
                    nextChar();
                    optional = true;
                    def = parseValue();
                    break;
                case '~':
                    nextChar();
                    origin = parseValue();
                    break;
                default:
                    loop = false;
                    break;
            }
        }
        return Argument {name, type, optional, def, origin, enumname};
    }

    Command parseScheme(executor_func executor) {
        std::string name = parseIdentifier();
        std::vector<Argument> args;
        std::unordered_map<std::string, Argument> kwargs;

        std::cout << "command.name: " << name << std::endl;
        while (hasNext()) {
            if (peek() == '{') {
                nextChar();
                while (peek() != '}') {
                    Argument arg = parseArgument();
                    kwargs[arg.name] = arg;
                }
                nextChar();
            } else {
                args.push_back(parseArgument());
            }
        }
        return Command(name, std::move(args), std::move(kwargs), executor);
    }

    CommandInput parsePrompt() {
        std::string name = parseIdentifier();
        auto args = dynamic::create_list();
        auto kwargs = dynamic::create_map();

        while (hasNext()) {
            auto value = parseValue();
            if (hasNext() && peek() == '=') {
                auto key = std::get<std::string>(value);
                nextChar();
                kwargs->put(key, parseValue());
            }
            args->put(value);
        }

        return CommandInput {name, args, kwargs};
    }
};

Command Command::create(std::string_view scheme, executor_func executor) {
    return CommandParser("<string>", scheme).parseScheme(executor);
}

void CommandsRepository::add(std::string_view scheme, executor_func executor) {
    Command command = Command::create(scheme, executor);
    commands[command.getName()] = command;
}

Command* CommandsRepository::get(const std::string& name) {
    auto found = commands.find(name);
    if (found == commands.end()) {
        return nullptr;
    }
    return &found->second;
}

CommandInput CommandsInterpreter::parse(std::string_view text) {
    return CommandParser("<string>", text).parsePrompt();
}
