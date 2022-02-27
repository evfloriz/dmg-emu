#include "PPU.h"

PPU::PPU() {

}

PPU::~PPU() {

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