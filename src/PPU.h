#pragma once

#include <cstdint>

#include "Cartridge.h"

class Bus;

class PPU {
public:
	PPU(Bus* bus);
	~PPU();

public:
	void connectBus(Bus* n) { bus = n; };
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	void clock();
	void updateLY();

	void updateTileData(uint32_t* tileDataBuffer);
	void updateTileMap(uint32_t* pixelBuffer);

	uint8_t getSCY();
	uint8_t getSCX();

	uint32_t* getPixelBuffer();
	uint32_t* getTileDataBuffer();

	bool frame_complete = false;

private:
	Bus* bus = nullptr;
	uint32_t palette[4];

	uint16_t cycle = 0;
	uint8_t scanline = 0;

	bool lcdc3 = false;
	bool lcdc4 = false;
	bool lcdc5 = false;
	bool lcdc6 = false;
	bool lcdc7 = false;

	uint32_t* pixelBuffer = nullptr;
	uint32_t* tileDataBuffer = nullptr;
};
