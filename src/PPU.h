#pragma once

#include <cstdint>

#include "Cartridge.h"

class MMU;

class PPU {
public:
	PPU(MMU* mmu);
	~PPU();

public:
	void clock();
	void updateLY();

	void updateTileData();
	void updateTileMaps();
	void updateScanline();
	void updateObjects();

	void setLine(uint32_t* buffer, uint16_t width, uint8_t x, uint8_t y, uint8_t hi, uint8_t lo);
	void setTile(
		uint32_t* buffer,
		uint16_t width,
		uint16_t start,
		uint16_t index,
		uint8_t x,
		uint8_t y);

	void setObject(
		uint32_t* buffer,
		uint16_t width,
		uint16_t start,
		uint16_t index,
		uint8_t x,
		uint8_t y,
		uint32_t* obp,
		bool xFlip,
		bool yFlip);

	uint32_t* getScreenBuffer();
	uint32_t* getTileDataBuffer();
	uint32_t* getBackgroundBuffer();
	uint32_t* getWindowBuffer();
	uint32_t* getObjectsBuffer();

	bool frameComplete = false;

private:
	MMU* mmu = nullptr;
	uint32_t palette[4];

	uint16_t cycle = 0;
	uint8_t scanline = 0;

	uint32_t* screenBuffer = new uint32_t[160 * 144];
	uint32_t* tileDataBuffer = new uint32_t[128 * 192];
	uint32_t* backgroundBuffer = new uint32_t[256 * 256];
	uint32_t* windowBuffer = new uint32_t[256 * 256];
	uint32_t* objectsBuffer = new uint32_t[256 * 256];
};
