#include "Bus.h"

#include "PPU.h"

PPU::PPU() {
	// TODO: check most logical order for these
	palette_screen[0] = olc::Pixel(15, 56, 15);
	palette_screen[1] = olc::Pixel(48, 98, 48);
	palette_screen[2] = olc::Pixel(139, 172, 139);
	palette_screen[3] = olc::Pixel(155, 188, 155);
}

PPU::~PPU() {

}

void PPU::write(uint16_t addr, uint8_t data) {
	delete sprite_screen;
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

void PPU::clock() {
	int random_colour = rand() % 4;
	sprite_screen->SetPixel(cycle - 1, scanline, palette_screen[random_colour]);
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
	
	// Reset to false every frame
	frame_complete = false;

	// Increment every 456 real clock cycles
	cycle++;
	if (cycle > 455) {
		cycle = 0;

		inc = true;
	}

	if (inc) {
		scanline = bus->read(0xFF44);

		// reset after 154 cycles
		scanline++;
		if (scanline > 153) {
			scanline = 0x00;
			frame_complete = true;
		}

		bus->write(0xFF44, scanline);
	}
}

bool PPU::frameComplete() {
	return frame_complete;
}