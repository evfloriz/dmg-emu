#pragma once

#include <cstdint>

class PPU {
public:
	PPU();
	~PPU();

public:
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	void clock();
};

