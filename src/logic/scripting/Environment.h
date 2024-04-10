#ifndef LOGIC_SCRIPTING_ENVIRONMENT_H_
#define LOGIC_SCRIPTING_ENVIRONMENT_H_

namespace scripting {
    /// @brief Lua environment wrapper for automatic deletion
    class Environment {
        int env;
    public:
        Environment(int env);
        ~Environment();

        int getId() const;

        // @brief Release namespace control
        void release();
    };
}

#endif // LOGIC_SCRIPTING_ENVIRONMENT_H_
