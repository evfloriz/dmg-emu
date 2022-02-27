#pragma once

#include <string>
#include <vector>

class Cartridge {
public:
	Cartridge(const std::string& fileName);
	~Cartridge();

public:
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);
};

