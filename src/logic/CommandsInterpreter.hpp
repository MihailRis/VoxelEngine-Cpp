#ifndef LOGIC_COMMANDS_INTERPRETER_HPP_
#define LOGIC_COMMANDS_INTERPRETER_HPP_

#include "../data/dynamic.hpp"

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace cmd {
    enum class ArgType {
        number, integer, enumvalue, selector, string
    };

    struct Argument {
        std::string name;
        ArgType type;
        bool optional;
        dynamic::Value def;
        dynamic::Value origin;
        std::string enumname;
    };

    class Command;

    struct Prompt {
        Command* command;
        dynamic::List_sptr args; // positional arguments list
        dynamic::Map_sptr kwargs; // keyword arguments table
    };

    using executor_func = std::function<dynamic::Value(
        dynamic::List_sptr args,
        dynamic::Map_sptr kwargs
    )>;

    class Command {
        std::string name;
        std::vector<Argument> args;
        std::unordered_map<std::string, Argument> kwargs;
        executor_func executor;
    public:
        Command() {}

        Command(
            std::string name,
            std::vector<Argument> args,
            std::unordered_map<std::string, Argument> kwargs,
            executor_func executor
        ) : name(name), 
            args(std::move(args)), 
            kwargs(std::move(kwargs)), 
            executor(executor) {}
        
        Argument* getArgument(size_t index) {
            if (index >= args.size())
                return nullptr;
            return &args[index];
        }

        Argument* getArgument(const std::string& keyword) {
            auto found = kwargs.find(keyword);
            if (found == kwargs.end()) {
                return nullptr;
            }
            return &found->second;
        }

        dynamic::Value execute(const Prompt& prompt) {
            return executor(prompt.args, prompt.kwargs);
        }

        const std::string& getName() const {
            return name;
        }

        static Command create(std::string_view scheme, executor_func);
    };

    class CommandsRepository {
        std::unordered_map<std::string, Command> commands;
    public:
        void add(std::string_view scheme, executor_func);
        Command* get(const std::string& name);
    };

    class CommandsInterpreter {
        std::unique_ptr<CommandsRepository> repository;
        std::unordered_map<std::string, dynamic::Value> variables;
    public:
        CommandsInterpreter(std::unique_ptr<CommandsRepository> repository)
        : repository(std::move(repository)){}

        Prompt parse(std::string_view text);

        dynamic::Value& operator[](const std::string& name) {
            return variables[name];
        }

        CommandsRepository* getRepository() const {
            return repository.get();
        }
    };
}

#endif // LOGIC_COMMANDS_INTERPRETER_HPP_
