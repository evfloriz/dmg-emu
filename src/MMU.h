#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "Cartridge.h"

class MMU {
public:
	MMU();
	~MMU();

public:
	std::shared_ptr<Cartridge> cart;

	// Main memory, bottom half is unused but simplifies addressing
	uint8_t* memory = new uint8_t[64 * 1024];

public:
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	// Lower overhead direct MMU read and write
	void directWrite(uint16_t addr, uint8_t data);
	uint8_t directRead(uint16_t addr);

	void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
};
