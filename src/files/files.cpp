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

// returns decompressed length
size_t decompressRLE(const ubyte* src, size_t length, ubyte* dst, size_t targetLength){
	size_t offset = 0;
	for (size_t i = 0; i < length;){
		unsigned char counter = src[i++];
		unsigned char c = src[i++];
		for (unsigned int j = 0; j <= counter; j++){
			dst[offset++] = c;
		}
	}
	return offset;
}

size_t calcRLE(const ubyte* src, size_t length) {
	size_t offset = 0;
	size_t counter = 0;
	ubyte c = src[0];
	for (size_t i = 0; i < length; i++){
		ubyte cnext = src[i];
		if (cnext != c || counter == 255){
			offset += 2;
			c = cnext;
			counter = 0;
		} else {
			counter++;
		}
	}
	return offset + 2;
}

// max result size = length * 2; returns compressed length
size_t compressRLE(const ubyte* src, size_t length, ubyte* dst) {
	if (length == 0)
		return 0;
	size_t offset = 0;
	uint counter = 0;
	ubyte c = src[0];
	for (size_t i = 1; i < length; i++){
		ubyte cnext = src[i];
		if (cnext != c || counter == 255){
			dst[offset++] = counter;
			dst[offset++] = c;
			c = cnext;
			counter = 0;
		} else {
			counter++;
		}
	}
	dst[offset++] = counter;
	dst[offset++] = c;
	return offset;
}
