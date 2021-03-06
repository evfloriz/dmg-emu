#include <fstream>

#include "Cartridge.h"

Cartridge::Cartridge(const std::string& fileName) {	
	// Read header to determine MBC type and size
	struct Header {
		uint8_t mbcType;
		uint8_t romSize;
		uint8_t ramSize;
	} header;
	
	// Load file into rom array
	std::ifstream ifs;
	ifs.open(fileName, std::ifstream::binary);
	if (ifs.is_open()) {
		// Read header
		ifs.seekg(0x147, ifs.beg);
		ifs.read((char*)&header, 3);

		// Resize rom to the correct size
		romBanks = 2 << header.romSize;
		romSize = (16 * 1024) * romBanks;
		rom.resize(romSize);

		// Resize ram to the correct size
		if (header.ramSize == 0x02) {
			ramBanks = 1;
		}
		else if (header.ramSize == 0x03) {
			ramBanks = 4;
		}
		else if (header.ramSize == 0x04) {
			ramBanks = 16;
		}
		else if (header.ramSize == 0x05) {
			ramBanks = 8;
		}
		ramSize = 8 * 1024 * ramBanks;
		ram.resize(ramSize);
		

		ifs.seekg(0, ifs.beg);
		ifs.read((char*)rom.data(), romSize);
		ifs.close();

		if (header.mbcType == 0x00) {
			mbc = std::make_shared<MBC0>();
		}
		else if (header.mbcType >= 0x01 && header.mbcType <= 0x05) {
			mbc = std::make_shared<MBC1>(romBanks, ramBanks);
		}
		else if (header.mbcType >= 0x0F && header.mbcType <= 0x13) {
			mbc = std::make_shared<MBC3>(romBanks, ramBanks);
		}
		else {
			printf("MBC not implemented");
		}
	}
	else {
		// TODO: Convert couts to printfs and add vita debug printf macro
		printf("Error loading rom\n");
	}
}

Cartridge::~Cartridge() {
}

uint8_t Cartridge::readRom(uint16_t addr) {
	uint32_t mappedAddr = mbc->mapRomAddr(addr);
	if (mappedAddr == 0xFFFFFFFF) {
		return 0xFF;
	}
	
	return rom[mappedAddr];
}

uint8_t Cartridge::readRam(uint16_t addr) {
	uint32_t mappedAddr = mbc->mapRamAddr(addr);
	if (mappedAddr == 0xFFFFFFFF) {
		return 0xFF;
	}

	return ram[mappedAddr];
}

void Cartridge::writeRam(uint16_t addr, uint8_t data) {
	if (ramSize == 0) {
		return;
	}

	uint32_t mappedAddr = mbc->mapRamAddr(addr);
	if (mappedAddr == 0xFFFFFFFF) {
		return;
	}

	ram[mappedAddr] = data;
}

void Cartridge::setRegister(uint16_t addr, uint8_t data) {
	// Change the state of the mapper chip to influence future reads
	mbc->setRegister(addr, data);
}
