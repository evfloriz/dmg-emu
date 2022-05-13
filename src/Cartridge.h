#pragma once

#include <string>
#include <vector>

#include "MBC0.h"
#include "MBC1.h"

class Cartridge {
public:
	Cartridge(const std::string& fileName);
	~Cartridge();

public:
	uint8_t read(uint16_t addr);
	void write(uint16_t addr, uint8_t data);
	void setRegister(uint16_t addr, uint8_t data);

private:
	std::vector<uint8_t> rom;
	std::vector<uint8_t> ram;

	// Initialized to default (no MBC) sizes
	uint32_t romSize = 32 * 1024;
	uint32_t ramSize = 0;
	uint8_t romBanks = 2;
	uint8_t ramBanks = 0;

	//uint8_t header[3] = {};

	std::shared_ptr<MBC> mbc;
};
