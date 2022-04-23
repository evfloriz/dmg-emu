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
	olc::Sprite* getTileMap(int map_num);

	void clock();
	void updateLY();

	void updateTileData();
	//void updateTileMap();
	void updateTileMap(uint32_t* pixelBuffer);

	uint8_t getSCY();
	uint8_t getSCX();

	uint32_t* getPixelBuffer();

	bool frame_complete = false;

private:
	Bus* bus = nullptr;
	//olc::Pixel palette[4];
	uint32_t palette[4];
	olc::Sprite* sprite_screen = new olc::Sprite(160, 144);
	
	olc::Sprite* block0 = new olc::Sprite(128, 64);
	olc::Sprite* block1 = new olc::Sprite(128, 64);
	olc::Sprite* block2 = new olc::Sprite(128, 64);

	olc::Sprite* bg_map = new olc::Sprite(256, 256);
	olc::Sprite* win_map = new olc::Sprite(256, 256);

	uint16_t cycle = 0;
	uint8_t scanline = 0;

	bool lcdc3 = false;
	bool lcdc4 = false;
	bool lcdc5 = false;
	bool lcdc6 = false;
	bool lcdc7 = false;

	uint32_t* pixelBuffer = nullptr;
};
