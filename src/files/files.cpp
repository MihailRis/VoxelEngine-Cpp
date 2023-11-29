#include "files.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdexcept>

using std::ios;
using std::string;
using std::unique_ptr;
using std::ifstream;
using std::ofstream;
using std::filesystem::path;

bool files::write_bytes(path filename, const char* data, size_t size) {
	ofstream output(filename, ios::binary);
	if (!output.is_open())
		return false;
	output.write(data, size);
	output.close();
	return true;
}

uint files::append_bytes(path filename, const char* data, size_t size) {
	ofstream output(filename, ios::binary | ios::app);
	if (!output.is_open())
		return 0;
	uint position = output.tellp();
	output.write(data, size);
	output.close();
	return position;
}

bool files::read(path filename, char* data, size_t size) {
	ifstream output(filename, ios::binary);
	if (!output.is_open())
		return false;
	output.read(data, size);
	output.close();
	return true;
}

char* files::read_bytes(path filename, size_t& length) {
	ifstream input(filename, ios::binary);
	if (!input.is_open())
		return nullptr;
	input.seekg(0, std::ios_base::end);
	length = input.tellg();
	input.seekg(0, std::ios_base::beg);

	unique_ptr<char> data {new char[length]};
	input.read(data.get(), length);
	input.close();
	return data.release();
}

std::string files::read_string(path filename) {
	size_t size;
	unique_ptr<char> chars (read_bytes(filename, size));
	if (chars == nullptr) {
		throw std::runtime_error("could not to load file '"+filename.string()+"'");
	}
	return string(chars.get(), size);
}

bool files::write_string(path filename, const string content) {
	ofstream file(filename);
	if (!file) {
		return false;
	}
	file << content;
	return true;
}