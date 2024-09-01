#pragma once
#ifdef DEBUG
	#define Log(x) std::cout<<x<<std::endl;
#else
	#define Log(x) 
#endif // DEBUG

#include <vector>
#include <string>
#include <fstream>
#define RTE(x) throw std::runtime_error(x);

namespace doll {

	std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
	std::string readFileString(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to read file!");
		}

		auto size = file.tellg();
		std::string content;
		content.resize(size);
		file.seekg(0);
		file.read(content.data(), content.size());

		return content;
	}

	
}