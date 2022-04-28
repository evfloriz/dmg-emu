#include <iostream>

#include "MMU.h"

#include "PPU.h"

// TODO: Make a util class.

uint32_t ARGB(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha = 255) {
	return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

PPU::PPU(MMU* mmu) {
	this->mmu = mmu;

	palette[0] = ARGB(155, 188, 155);
	palette[1] = ARGB(139, 172, 139);
	palette[2] = ARGB(48, 98, 48);
	palette[3] = ARGB(15, 56, 15);
}

PPU::~PPU() {
}

void PPU::write(uint16_t addr, uint8_t data) {
}

uint8_t PPU::read(uint16_t addr) {	
	uint8_t data = 0x00;

	// Handle data in each of the 12 registers of the ppu
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

void PPU::clock() {
	// Update LY
	updateLY();
}

void PPU::updateLY() {
	// TODO: does it make sense for the PPU to read random areas of memory other than vram and oam?
	// First check if screen is off and reset everything if so
	if (!(mmu->read(0xFF40) & 0x80)) {
		// LCD off
		mmu->write(0xFF44, 0x00);
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
		scanline = mmu->read(0xFF44);

		// Reset after 154 cycles
		scanline++;
		
		// Set VBLANK interrupt flag when LY is 144
		if (scanline == 144) {
			mmu->write(0xFF0F, (1 << 0));
		}

		if (scanline > 153) {
			scanline = 0x00;
			frame_complete = true;
		}

		mmu->write(0xFF44, scanline);
	}
}

void PPU::updateTileData(uint32_t* tileDataBuffer) {
	uint16_t block0_start = 0x8000;
	uint16_t block1_start = 0x8800;
	uint16_t block2_start = 0x9000;

	auto set_line = [&](uint8_t x, uint8_t y, uint8_t hi, uint8_t lo) {
		// Get palette index from leftmost to rightmost pixel
		tileDataBuffer[y * 256 + x    ] = palette[((hi >> 6) & (1 << 1)) | ((lo >> 7) & 1)];
		tileDataBuffer[y * 256 + x + 1] = palette[((hi >> 5) & (1 << 1)) | ((lo >> 6) & 1)];
		tileDataBuffer[y * 256 + x + 2] = palette[((hi >> 4) & (1 << 1)) | ((lo >> 5) & 1)];
		tileDataBuffer[y * 256 + x + 3] = palette[((hi >> 3) & (1 << 1)) | ((lo >> 4) & 1)];

		tileDataBuffer[y * 256 + x + 4] = palette[((hi >> 2) & (1 << 1)) | ((lo >> 3) & 1)];
		tileDataBuffer[y * 256 + x + 5] = palette[((hi >> 1) & (1 << 1)) | ((lo >> 2) & 1)];
		tileDataBuffer[y * 256 + x + 6] = palette[((hi >> 0) & (1 << 1)) | ((lo >> 1) & 1)];
		tileDataBuffer[y * 256 + x + 7] = palette[((hi << 1) & (1 << 1)) | ((lo >> 0) & 1)];
	};

	for (int i = 0; i < 0x0080; i++) {
		uint8_t x = i % 16;
		uint8_t y = i / 16;

		for (int j = 0; j < 8; j++) {
			uint8_t lo0 = mmu->read(block0_start + i * 16 + j * 2);
			uint8_t hi0 = mmu->read(block0_start + i * 16 + j * 2 + 1);

			uint8_t lo1 = mmu->read(block1_start + i * 16 + j * 2);
			uint8_t hi1 = mmu->read(block1_start + i * 16 + j * 2 + 1);

			uint8_t lo2 = mmu->read(block2_start + i * 16 + j * 2);
			uint8_t hi2 = mmu->read(block2_start + i * 16 + j * 2 + 1);

			set_line(x * 8, j + y * 8, hi0, lo0);
			set_line(x * 8, 64 + j + y * 8, hi1, lo1);
			set_line(x * 8, 128 + j + y * 8, hi2, lo2);
		}
	}
}

void PPU::updateTileMap(uint32_t* pixelBuffer) {
	// Check LCD control
	lcdc4 = mmu->read(0xFF40) & (1 << 4);
	lcdc6 = mmu->read(0xFF40) & (1 << 6);
	lcdc3 = mmu->read(0xFF40) & (1 << 3);
	lcdc5 = mmu->read(0xFF40) & (1 << 5);
	lcdc7 = mmu->read(0xFF40) & (1 << 7);

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

	// Optimization: Use scaleX and scaleY to set the start offset, and then only add the actual screens pixels
	// to pixelbuffer. Treat it like the screen rather than the background map
	
	auto set_line = [&](uint8_t x, uint8_t y, uint8_t hi, uint8_t lo) {
		// Get palette index from leftmost to rightmost pixel
		pixelBuffer[y * 256 + x    ] = palette[((hi >> 6) & (1 << 1)) | ((lo >> 7) & 1)];
		pixelBuffer[y * 256 + x + 1] = palette[((hi >> 5) & (1 << 1)) | ((lo >> 6) & 1)];
		pixelBuffer[y * 256 + x + 2] = palette[((hi >> 4) & (1 << 1)) | ((lo >> 5) & 1)];
		pixelBuffer[y * 256 + x + 3] = palette[((hi >> 3) & (1 << 1)) | ((lo >> 4) & 1)];

		pixelBuffer[y * 256 + x + 4] = palette[((hi >> 2) & (1 << 1)) | ((lo >> 3) & 1)];
		pixelBuffer[y * 256 + x + 5] = palette[((hi >> 1) & (1 << 1)) | ((lo >> 2) & 1)];
		pixelBuffer[y * 256 + x + 6] = palette[((hi >> 0) & (1 << 1)) | ((lo >> 1) & 1)];
		pixelBuffer[y * 256 + x + 7] = palette[((hi << 1) & (1 << 1)) | ((lo >> 0) & 1)];
	};

	for (int i = 0; i < 0x0400; i++) {
		uint8_t x = i % 32;
		uint8_t y = i / 32;

		uint8_t index = mmu->read(bg_start + i);
		uint16_t start = (index > 127) ? second_half_start : first_half_start;
		
		// TODO: clean up logic
		// Read each pair of bytes and set each of the 8 lines of pixels
		for (int j = 0; j < 8; j++) {
			uint8_t lo = mmu->read(start + (index % 128) * 16 + j * 2);
			uint8_t hi = mmu->read(start + (index % 128) * 16 + j * 2 + 1);

			set_line(x * 8, j + y * 8, hi, lo);
		}
	}
}

uint8_t PPU::getSCY() {
	return mmu->read(0xFF42);
}

uint8_t PPU::getSCX() {
	return mmu->read(0xFF43);
}

uint32_t* PPU::getPixelBuffer() {
	return pixelBuffer;
}

uint32_t* PPU::getTileDataBuffer() {
	return tileDataBuffer;
}
