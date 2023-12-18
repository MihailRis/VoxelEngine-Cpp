#include "files.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdexcept>
#include "../coders/json.h"

namespace fs = std::filesystem;

bool files::write_bytes(fs::path filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary);
	if (!output.is_open())
		return false;
	output.write(data, size);
	output.close();
	return true;
}

uint files::append_bytes(fs::path filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary | std::ios::app);
	if (!output.is_open())
		return 0;
	uint position = output.tellp();
	output.write(data, size);
	output.close();
	return position;
}

bool files::read(fs::path filename, char* data, size_t size) {
	std::ifstream output(filename, std::ios::binary);
	if (!output.is_open())
		return false;
	output.read(data, size);
	output.close();
	return true;
}

char* files::read_bytes(fs::path filename, size_t& length) {
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open())
		return nullptr;
	input.seekg(0, std::ios_base::end);
	length = input.tellg();
	input.seekg(0, std::ios_base::beg);

	std::unique_ptr<char> data(new char[length]);
	input.read(data.get(), length);
	input.close();
	return data.release();
}

std::string files::read_string(fs::path filename) {
	size_t size;
	std::unique_ptr<char> chars (read_bytes(filename, size));
	if (chars == nullptr) {
		throw std::runtime_error("could not to load file '"+
								 filename.string()+"'");
	}
	return std::string(chars.get(), size);
}

bool files::write_string(fs::path filename, const std::string content) {
	std::ofstream file(filename);
	if (!file) {
		return false;
	}
	file << content;
	return true;
}

json::JObject* files::read_json(fs::path file) {
	std::string text = files::read_string(file);
	try {
		return json::parse(file.string(), text);
	} catch (const parsing_error& error) {
        std::cerr << error.errorLog() << std::endl;
        throw std::runtime_error("could not to parse "+file.string());
    }
}

std::vector<std::string> files::read_list(std::filesystem::path filename) {
	std::ifstream file(filename);
	if (!file) {
		throw std::runtime_error("could not to open file "+filename.u8string());
	}
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line)) {
		if (line.length() == 0)
			continue;
		if (line[0] == '#')
			continue;
		lines.push_back(line);
	}
	return lines;
}
