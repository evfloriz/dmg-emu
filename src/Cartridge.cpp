#include <fstream>

#include "Cartridge.h"


Cartridge::Cartridge(const std::string& fileName) {	
	// Load file into rom array
	std::ifstream ifs;
	ifs.open(fileName, std::ifstream::binary);
	if (ifs.is_open()) {
		ifs.read((char*)rom, 32 * 1024);
		ifs.close();
	}
}

Cartridge::~Cartridge() {
	delete[] rom;
}

uint8_t Cartridge::read(uint16_t addr) {
	return rom[addr];
}
