#ifndef UTIL_COMMAND_LINE_HPP_
#define UTIL_COMMAND_LINE_HPP_

class EnginePaths;

/// @return false if engine start can
bool parse_cmdline(int argc, char** argv, EnginePaths& paths);

#endif // UTIL_COMMAND_LINE_HPP_
