#pragma once

#include <cstdint>

#include "Cartridge.h"

class MMU;

class PPU {
public:
	PPU(MMU* mmu);
	~PPU();

public:
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	void clock();
	void updateLY();

	void updateTileData(uint32_t* buffer);
	void updateTileMap(uint32_t* buffer, uint16_t start);
	void updateBackgroundTileMap(uint32_t* buffer);
	void updateWindowTileMap(uint32_t* buffer);
	void updateScreen(uint32_t* screenBuffer);

	uint32_t* getScreenBuffer();
	uint32_t* getTileDataBuffer();
	uint32_t* getBackgroundBuffer();
	uint32_t* getWindowBuffer();

	bool frameComplete = false;

private:
	MMU* mmu = nullptr;
	uint32_t palette[4];

	uint16_t cycle = 0;
	uint8_t scanline = 0;

	bool lcdc3 = false;
	bool lcdc4 = false;
	bool lcdc5 = false;
	bool lcdc6 = false;
	bool lcdc7 = false;

	uint32_t* screenBuffer = nullptr;
	uint32_t* tileDataBuffer = nullptr;
	uint32_t* backgroundBuffer = nullptr;
	uint32_t* windowBuffer = nullptr;
};
