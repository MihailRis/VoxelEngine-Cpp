#pragma once

struct CoreParameters;

/// @return false if engine start can
bool parse_cmdline(int argc, char** argv, CoreParameters& params);
