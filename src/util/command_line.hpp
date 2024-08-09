#pragma once

class EnginePaths;

/// @return false if engine start can
bool parse_cmdline(int argc, char** argv, EnginePaths& paths);
