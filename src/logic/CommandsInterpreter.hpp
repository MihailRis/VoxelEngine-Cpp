#ifndef LOGIC_COMMANDS_INTERPRETER_HPP_
#define LOGIC_COMMANDS_INTERPRETER_HPP_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "data/dynamic.hpp"

namespace cmd {
    enum class ArgType { number, integer, enumvalue, selector, string };

    inline std::string argtype_name(ArgType type) {
        switch (type) {
            case ArgType::number:
                return "number";
            case ArgType::integer:
                return "integer";
            case ArgType::enumvalue:
                return "enumeration";
            case ArgType::selector:
                return "selector";
            case ArgType::string:
                return "string";
            default:
                return "<unknown>";
        }
    }

    struct Argument {
        std::string name;
        ArgType type;
        bool optional;
        dynamic::Value def;
        dynamic::Value origin;
        std::string enumname;
    };

    class Command;
    class CommandsInterpreter;

    struct Prompt {
        Command* command;
        dynamic::List_sptr args;   // positional arguments list
        dynamic::Map_sptr kwargs;  // keyword arguments table
    };

    using executor_func = std::function<dynamic::Value(
        CommandsInterpreter*, dynamic::List_sptr args, dynamic::Map_sptr kwargs
    )>;

    class Command {
        std::string name;
        std::vector<Argument> args;
        std::unordered_map<std::string, Argument> kwargs;
        std::string description;
        executor_func executor;
    public:
        Command() = default;

        Command(
            std::string name,
            std::vector<Argument> args,
            std::unordered_map<std::string, Argument> kwargs,
            std::string description,
            executor_func executor
        )
            : name(name),
              args(std::move(args)),
              kwargs(std::move(kwargs)),
              description(description),
              executor(executor) {
        }

        Argument* getArgument(size_t index) {
            if (index >= args.size()) return nullptr;
            return &args[index];
        }

        Argument* getArgument(const std::string& keyword) {
            auto found = kwargs.find(keyword);
            if (found == kwargs.end()) {
                return nullptr;
            }
            return &found->second;
        }

        dynamic::Value execute(
            CommandsInterpreter* interpreter, const Prompt& prompt
        ) {
            return executor(interpreter, prompt.args, prompt.kwargs);
        }

        const std::string& getName() const {
            return name;
        }

        const std::vector<Argument>& getArgs() const {
            return args;
        }

        const std::unordered_map<std::string, Argument>& getKwArgs() const {
            return kwargs;
        }

        const std::string& getDescription() const {
            return description;
        }

        static Command create(
            std::string_view scheme, std::string_view description, executor_func
        );
    };

    class CommandsRepository {
        std::unordered_map<std::string, Command> commands;
    public:
        void add(
            std::string_view scheme, std::string_view description, executor_func
        );
        Command* get(const std::string& name);

        const std::unordered_map<std::string, Command>& getCommands() const {
            return commands;
        }
    };

    class CommandsInterpreter {
        std::unique_ptr<CommandsRepository> repository;
        std::unordered_map<std::string, dynamic::Value> variables;
    public:
        CommandsInterpreter()
            : repository(std::make_unique<CommandsRepository>()) {
        }

        CommandsInterpreter(const CommandsInterpreter&) = delete;

        CommandsInterpreter(std::unique_ptr<CommandsRepository> repository)
            : repository(std::move(repository)) {
        }

        Prompt parse(std::string_view text);

        dynamic::Value execute(std::string_view input) {
            return execute(parse(input));
        }

        dynamic::Value execute(const Prompt& prompt) {
            return prompt.command->execute(this, prompt);
        }

        dynamic::Value& operator[](const std::string& name) {
            return variables[name];
        }

        CommandsRepository* getRepository() const {
            return repository.get();
        }
    };
}

#endif  // LOGIC_COMMANDS_INTERPRETER_HPP_
