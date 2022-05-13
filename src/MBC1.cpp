#include "MBC1.h"

uint32_t MBC1::mapAddr(uint16_t addr) {
	// Return the mapped address to read from given the current state of the MBC
	if (addr <= 0x3FFF) {
		return addr;
	}
	else if (addr <= 0x7FFF) {
		return romBankNumber * ROM_BANK_SIZE + (addr - 0x4000);
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		// Return address of ram to use in the cartridge's ram vector
		if (ramEnable) {
			return ramBankNumber * RAM_BANK_SIZE + (addr - 0xA000);
		}
		else {
			return 0xFFFFFFFF;
		}
		
	}
	else {
		// This is an error, shouldn't get here
		return addr;
	}
}

void MBC1::setRegister(uint16_t addr, uint8_t data) {
	// Modify the MBC state
	if (addr <= 0x1FFF) {
		// Ram enable
		data &= 0x0A;
		ramEnable = data;
	}
	else if (addr <= 0x3FFF) {
		// Rom bank number, bottom 5 bits
		data &= 0x1F;
		if (data == 0x00) {
			data = 0x01;
		}
		romBankNumber = data;
	}
	else if (addr <= 0x5FFF) {
		data &= 0x03;
		if (modeSelect == 0x01) {
			ramBankNumber = data;
		}
		else {
			upperRomBankNumber = data;
		}
	}
	else if (addr <= 0x7FFF) {
		data &= 0x01;
		modeSelect = data;
	}
}
