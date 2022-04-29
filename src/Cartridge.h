#pragma once

#include <string>
#include <array>

class Cartridge {
public:
	Cartridge(const std::string& fileName);
	~Cartridge();

public:
	uint8_t* rom = new uint8_t[32 * 1024];

public:
	uint8_t read(uint16_t addr);
};
