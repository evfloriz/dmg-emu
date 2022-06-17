#pragma once

#include <string>
#include <vector>
#include <memory>

#include "MBC0.h"
#include "MBC1.h"
#include "MBC3.h"

class Cartridge {
public:
	Cartridge(const std::string& fileName);
	~Cartridge();

public:
	uint8_t readRom(uint16_t addr);
	uint8_t readRam(uint16_t addr);
	void writeRam(uint16_t addr, uint8_t data);
	void setRegister(uint16_t addr, uint8_t data);

private:
	std::vector<uint8_t> rom;
	std::vector<uint8_t> ram;

	// Initialized to default (no MBC) sizes
	uint32_t romSize = 32 * 1024;
	uint32_t ramSize = 0;
	uint8_t romBanks = 2;
	uint8_t ramBanks = 0;

	std::shared_ptr<MBC> mbc;
};
