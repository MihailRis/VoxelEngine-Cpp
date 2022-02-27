#include "files.h"

#include <fstream>
#include <iostream>

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

	char* data = new char[length];
	input.read(data, length);
	input.close();
	return data;
}

// returns decompressed length
unsigned int decompressRLE(const char* src, unsigned int length, char* dst, unsigned int targetLength){
	unsigned int offset = 0;
	for (unsigned int i = 0; i < length;){
		unsigned char counter = src[i++];
		char c = src[i++];
		for (unsigned int j = 0; j <= counter; j++){
			dst[offset++] = c;
		}
	}
	return offset;
}

unsigned int calcRLE(const char* src, unsigned int length) {
	unsigned int offset = 0;
	unsigned int counter = 1;
	char c = src[0];
	for (unsigned int i = 0; i < length; i++){
		char cnext = src[i];
		if (cnext != c || counter == 256){
			offset += 2;
			c = cnext;
			counter = 0;
		}
		counter++;
	}
	return offset + 2;
}

// max result size = length * 2; returns compressed length
unsigned int compressRLE(const char* src, unsigned int length, char* dst) {
	unsigned int offset = 0;
	unsigned int counter = 1;
	char c = src[0];
	for (unsigned int i = 1; i < length; i++){
		char cnext = src[i];
		if (cnext != c || counter == 256){
			dst[offset++] = counter-1;
			dst[offset++] = c;
			c = cnext;
			counter = 0;
		}
		counter++;
	}
	dst[offset++] = counter-1;
	dst[offset++] = c;
	return offset;
}
