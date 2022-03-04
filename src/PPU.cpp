#include <iostream>

#include "Bus.h"

#include "PPU.h"

PPU::PPU() {
	// TODO: check most logical order for these
	palette[0] = olc::Pixel(155, 188, 155);
	palette[1] = olc::Pixel(139, 172, 139);
	palette[2] = olc::Pixel(48, 98, 48);
	palette[3] = olc::Pixel(15, 56, 15);
	
}

PPU::~PPU() {
	delete sprite_screen;

	delete block0;
	delete block1;
	delete block2;
}

void PPU::write(uint16_t addr, uint8_t data) {
}

uint8_t PPU::read(uint16_t addr) {	
	uint8_t data = 0x00;

	// handle data in each of the 12 registers of the ppu
	switch (addr) {
	case 0xFF40:
		break;
	case 0xFF41:
		break;
	case 0xFF42:
		break;
	case 0xFF43:
		break;
	case 0xFF44:
		break;
	case 0xFF45:
		break;
	case 0xFF46:
		break;
	case 0xFF47:
		break;
	case 0xFF48:
		break;
	case 0xFF49:
		break;
	case 0xFF4A:
		break;
	case 0xFF4B:
		break;
	}

	return data;
}

olc::Sprite* PPU::getScreen() {
	return sprite_screen;
}

olc::Sprite* PPU::getTileData(int block_num) {
	if (block_num == 0) {
		return block0;
	}
	else if (block_num == 1) {
		return block1;
	}
	else {
		return block2;
	}
}

olc::Sprite* PPU::getTileMap(int map_num) {
	if (map_num == 0) {
		return bg_map;
	}
	else {
		return win_map;
	}
}

void PPU::clock() {
	int random_colour = rand() % 4;
	sprite_screen->SetPixel(cycle - 1, scanline, palette[random_colour]);
	//sprite_screen->SetPixel(cycle - 1, scanline, palette_screen[(rand() % 4)]);

	// Update LY
	updateLY();
}

void PPU::updateLY() {
	// TODO: does it make sense for the PPU to read random areas of memory other than vram and oam?
	// First check if screen is off and reset everything if so
	if (!(bus->read(0xFF40) & 0x80)) {
		// LCD off
		bus->write(0xFF44, 0x00);
		cycle = 0;

		return;
	}

	bool inc = false;

	// Increment every 456 real clock cycles
	cycle++;
	if (cycle > 455) {
		cycle = 0;

		inc = true;
	}

	if (inc) {
		scanline = bus->read(0xFF44);

		// Reset after 154 cycles
		scanline++;
		
		// Set VBLANK interrupt flag when LY is 144
		if (scanline == 144) {
			bus->write(0xFF0F, (1 << 0));
		}

		if (scanline > 153) {
			scanline = 0x00;
			frame_complete = true;
		}

		bus->write(0xFF44, scanline);
	}
}

void PPU::updateTileData() {
	// First check LCD mode
	if (bus->read(0xFF40) & (1 << 4)) {
		lcdc4 = true;
	}
	else {
		lcdc4 = false;
	}

	// If lcdc4 = 1, block0 is 0-127 and block1 is 128-255
	// If lcdc4 = 0, block2 is 0-127 and block1 is 128-255
	uint16_t block0_start = 0x8000;
	uint16_t block1_start = 0x8800;
	uint16_t block2_start = 0x9000;

	auto set_line = [&](olc::Sprite* block, uint8_t x, uint8_t y, uint8_t hi, uint8_t lo) {
		// Get palette index from leftmost to rightmost pixel
		block->SetPixel(x, y,		palette[((hi >> 6) & (1 << 1)) | ((lo >> 7) & 1)]);
		block->SetPixel(x + 1, y,	palette[((hi >> 5) & (1 << 1)) | ((lo >> 6) & 1)]);
		block->SetPixel(x + 2, y,	palette[((hi >> 4) & (1 << 1)) | ((lo >> 5) & 1)]);
		block->SetPixel(x + 3, y,	palette[((hi >> 3) & (1 << 1)) | ((lo >> 4) & 1)]);

		block->SetPixel(x + 4, y,	palette[((hi >> 2) & (1 << 1)) | ((lo >> 3) & 1)]);
		block->SetPixel(x + 5, y,	palette[((hi >> 1) & (1 << 1)) | ((lo >> 2) & 1)]);
		block->SetPixel(x + 6, y,	palette[((hi >> 0) & (1 << 1)) | ((lo >> 1) & 1)]);
		block->SetPixel(x + 7, y,	palette[((hi << 1) & (1 << 1)) | ((lo >> 0) & 1)]);
	};

	uint8_t y_base = 0;
	uint8_t y = y_base;
	uint8_t x = 0;


	for (int i = 0; i < 0x0800; i += 2) {
		uint8_t lo0 = bus->read(block0_start + i);
		uint8_t hi0 = bus->read(block0_start + i + 1);

		uint8_t lo1 = bus->read(block1_start + i);
		uint8_t hi1 = bus->read(block1_start + i + 1);

		uint8_t lo2 = bus->read(block2_start + i);
		uint8_t hi2 = bus->read(block2_start + i + 1);

		// Set the line of pixels
		//std::cout << "writing x, y: " << x * 8 << ", " << y + y_base << std::endl;
		//std::cout << "block index: " << (block0_start + i) << std::endl;
		set_line(block0, x * 8, y + y_base, hi0, lo0);
		set_line(block1, x * 8, y + y_base, hi1, lo1);
		set_line(block2, x * 8, y + y_base, hi2, lo1);

		// TODO: refactor counting logic
		
		// Increment y after every line
		y++;

		// If y exceeds 8, the tile is done so increment x by 8 and set y to 0
		if (y > 7) {
			x++;
			y = 0;
		}

		// If x exceeds 16, row of tiles is done so increment y_base and reset x to 0
		if (x > 15) {
			x = 0;
			y_base += 8;
		}
	}

}

void PPU::updateTileDataTest() {
	// First check LCD mode
	if (bus->read(0xFF40) & (1 << 4)) {
		lcdc4 = true;
	}
	else {
		lcdc4 = false;
	}

	// If lcdc4 = 1, block0 is 0-127 and block1 is 128-255
	// If lcdc4 = 0, block2 is 0-127 and block1 is 128-255
	uint16_t block0_start = 0x8000;
	uint16_t block1_start = 0x8800;
	uint16_t block2_start = 0x9000;

	auto set_line = [&](olc::Sprite* block, uint8_t x, uint8_t y, uint8_t hi, uint8_t lo) {
		// Get palette index from leftmost to rightmost pixel
		block->SetPixel(x, y, palette[((hi >> 6) & (1 << 1)) | ((lo >> 7) & 1)]);
		block->SetPixel(x + 1, y, palette[((hi >> 5) & (1 << 1)) | ((lo >> 6) & 1)]);
		block->SetPixel(x + 2, y, palette[((hi >> 4) & (1 << 1)) | ((lo >> 5) & 1)]);
		block->SetPixel(x + 3, y, palette[((hi >> 3) & (1 << 1)) | ((lo >> 4) & 1)]);

		block->SetPixel(x + 4, y, palette[((hi >> 2) & (1 << 1)) | ((lo >> 3) & 1)]);
		block->SetPixel(x + 5, y, palette[((hi >> 1) & (1 << 1)) | ((lo >> 2) & 1)]);
		block->SetPixel(x + 6, y, palette[((hi >> 0) & (1 << 1)) | ((lo >> 1) & 1)]);
		block->SetPixel(x + 7, y, palette[((hi << 1) & (1 << 1)) | ((lo >> 0) & 1)]);
	};

	for (int i = 0; i < 0x0100; i++) {
		uint8_t x = i % 16;
		uint8_t y = i / 16;

		for (int j = 0; j < 8; j++) {
			uint8_t lo0 = bus->read(block0_start + i * 16 + j * 2);
			uint8_t hi0 = bus->read(block0_start + i * 16 + j * 2 + 1);

			uint8_t lo1 = bus->read(block1_start + i * 16 + j * 2);
			uint8_t hi1 = bus->read(block1_start + i * 16 + j * 2 + 1);

			uint8_t lo2 = bus->read(block2_start + i * 16 + j * 2);
			uint8_t hi2 = bus->read(block2_start + i * 16 + j * 2 + 1);

			set_line(block0, x * 8, j + y * 8, hi0, lo0);
			set_line(block1, x * 8, j + y * 8, hi1, lo1);
			set_line(block2, x * 8, j + y * 8, hi2, lo1);
		}
	}

}

void PPU::updateTileMap() {
	// Check LCD control
	lcdc4 = bus->read(0xFF40) & (1 << 4);
	lcdc6 = bus->read(0xFF40) & (1 << 6);
	lcdc3 = bus->read(0xFF40) & (1 << 3);
	lcdc5 = bus->read(0xFF40) & (1 << 5);
	lcdc7 = bus->read(0xFF40) & (1 << 7);

	// Early out if LCD is off
	if (!lcdc7) {
		return;
	}

	// If lcdc4 = 1, 0-127 starts at 0x8000 and 128-255 starts at 0x8800
	// If lcdc4 = 0, 0-127 starts at 0x9000 and 128-255 starts at 0x8800
	uint16_t first_half_start = lcdc4 ? 0x8000 : 0x9000;
	uint16_t second_half_start = 0x8800;

	// If lcdc3 = 1, bg map starts at 0x9C00, otherwise 0x9800
	uint16_t bg_start = lcdc3 ? 0x9C00 : 0x9800;

	// If lcdc6 = 1, win map starts at 0x9C00, otherwise 0x9800
	uint16_t win_start = lcdc6 ? 0x9C00 : 0x9800;
	
	auto set_line = [&](olc::Sprite* map, uint8_t x, uint8_t y, uint8_t hi, uint8_t lo) {
		// Get palette index from leftmost to rightmost pixel
		map->SetPixel(x, y,		palette[((hi >> 6) & (1 << 1)) | ((lo >> 7) & 1)]);
		map->SetPixel(x + 1, y, palette[((hi >> 5) & (1 << 1)) | ((lo >> 6) & 1)]);
		map->SetPixel(x + 2, y, palette[((hi >> 4) & (1 << 1)) | ((lo >> 5) & 1)]);
		map->SetPixel(x + 3, y, palette[((hi >> 3) & (1 << 1)) | ((lo >> 4) & 1)]);

		map->SetPixel(x + 4, y, palette[((hi >> 2) & (1 << 1)) | ((lo >> 3) & 1)]);
		map->SetPixel(x + 5, y, palette[((hi >> 1) & (1 << 1)) | ((lo >> 2) & 1)]);
		map->SetPixel(x + 6, y, palette[((hi >> 0) & (1 << 1)) | ((lo >> 1) & 1)]);
		map->SetPixel(x + 7, y, palette[((hi << 1) & (1 << 1)) | ((lo >> 0) & 1)]);
	};

	for (int i = 0; i < 0x0400; i++) {
		uint8_t x = i % 32;
		uint8_t y = i / 32;

		uint8_t index = bus->read(bg_start + i);
		uint16_t start = (index > 127) ? second_half_start : first_half_start;
		
		// TODO: cleanup logic
		// Read each pair of bytes and set each of the 8 lines of pixels
		for (int j = 0; j < 8; j++) {
			uint8_t lo = bus->read(start + (index % 128) * 16 + j * 2);
			uint8_t hi = bus->read(start + (index % 128) * 16 + j * 2 + 1);

			set_line(bg_map, x * 8, j + y * 8, hi, lo);
		}
	}
}

uint8_t PPU::getSCY() {
	return bus->read(0xFF42);
}

uint8_t PPU::getSCX() {
	return bus->read(0xFF43);
}