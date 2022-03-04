#pragma once

#include <cstdint>

#include "olcPixelGameEngine.h"

#include "Cartridge.h"

class Bus;

class PPU {
public:
	PPU();
	~PPU();

public:
	// Link CPU to bus
	void connectBus(Bus* n) { bus = n; };
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	olc::Sprite* getScreen();
	olc::Sprite* getTileData(int block_num);

	void clock();
	void updateLY();

	void updateTileData();

	bool frameComplete();

private:
	Bus* bus = nullptr;
	olc::Pixel palette[4];
	olc::Sprite* sprite_screen = new olc::Sprite(160, 144);
	
	olc::Sprite* block0 = new olc::Sprite(128, 64);
	olc::Sprite* block1 = new olc::Sprite(128, 64);
	olc::Sprite* block2 = new olc::Sprite(128, 64);

	uint16_t cycle = 0;
	uint8_t scanline = 0;

	bool frame_complete = false;

	bool lcdc4 = false;
};

