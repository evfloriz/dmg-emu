#pragma once

#include <cstdint>
#include <array>

#include "CPU.h"

class Bus {
public:
	Bus();
	~Bus();

public:
	// Devices on bus
	// CPU
	CPU cpu;

	// Fake RAM for now, 8KB
	std::array<uint8_t, 8 * 1024> ram;

public:
	// Bus read and write
	// Should be 16 bit addresses with 8 bit data, read one register at a time I think
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);
};