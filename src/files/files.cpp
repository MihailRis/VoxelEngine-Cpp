#include "files.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <stdint.h>

bool write_binary_file_part(std::string filename, const char* data, size_t offset, size_t size){
	std::ofstream output(filename, std::ios::out | std::ios::binary | std::ios::in);
	if (!output.is_open())
		return false;
	output.seekp(offset);
	output.write(data, size);
	return true;
}

bool write_binary_file(std::string filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary);
	if (!output.is_open())
		return false;
	output.write(data, size);
	output.close();
	return true;
}

unsigned int append_binary_file(std::string filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary | std::ios::app);
	if (!output.is_open())
		return 0;
	unsigned int position = output.tellp();
	output.write(data, size);
	output.close();
	return position;
}

bool read_binary_file(std::string filename, char* data, size_t size) {
	std::ifstream output(filename, std::ios::binary);
	if (!output.is_open())
		return false;
	output.read(data, size);
	output.close();
	return true;
}

char* read_binary_file(std::string filename, size_t& length) {
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open())
		return nullptr;
	input.seekg(0, std::ios_base::end);
	length = input.tellg();
	input.seekg(0, std::ios_base::beg);

	std::unique_ptr<char> data {new char[length]};
	input.read(data.get(), length);
	input.close();
	return data.release();
}
