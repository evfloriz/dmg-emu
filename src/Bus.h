#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "Cartridge.h"

class Bus {
public:
	Bus();
	~Bus();

public:
	// Devices on bus
	std::shared_ptr<Cartridge> cart;

	// Main memory, needs to be offset by 0x7FFF for proper addressing.
	uint8_t* memory = new uint8_t[32 * 1024];
	
	// Implement frequently used registers as ints for quicker access
	uint8_t ieRegister;
	uint8_t ifRegister;
	uint8_t timerControlRegister;


public:
	// Bus read and write
	// Should be 16 bit addresses with 8 bit data, read one register at a time I think
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
};
