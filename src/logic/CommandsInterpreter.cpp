#include "CommandsInterpreter.hpp"

#include "../coders/commons.hpp"
#include "../util/stringutil.hpp"

#include <iostream>

using namespace cmd;

inline bool is_cmd_identifier_part(char c) {
    return is_identifier_part(c) || c == '.' || c == '$';
}

class SchemeParser : BasicParser {
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
        {"selector", ArgType::selector},
    };
public:
    SchemeParser(std::string_view filename, std::string_view source) 
    : BasicParser(filename, source) {
    }

    ArgType parseType() {
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
        if (is_cmd_identifier_part(c)) {
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

    Argument parseArgument() {
        std::string name = parseIdentifier();
        expect(':');
        std::cout << "arg.name: " << name << std::endl;
        ArgType type = parseType();
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
        return Argument {name, type, optional, def, origin};
    }

    Command parse(executor_func executor) {
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
};

Command Command::create(std::string_view scheme, executor_func executor) {
    return SchemeParser("<string>", scheme).parse(executor);
}
