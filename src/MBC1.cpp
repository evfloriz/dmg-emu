#include "MBC1.h"

uint32_t MBC1::mapRomAddr(uint16_t addr) {
	if (addr <= 0x3FFF) {
		return addr;
	}
	else if (addr <= 0x7FFF) {
		// Use the upper rom bank number bits if in rom mode
		uint8_t newRomBankNumber = romBankNumber;
		if (modeSelect == 0x00) {
			newRomBankNumber = (upperRomBankNumber << 5) | romBankNumber;
		}
		return newRomBankNumber * ROM_BANK_SIZE + (addr - 0x4000);
	}
	else {
		// Error
		return 0xFFFFFFFF;
	}
}

uint32_t MBC1::mapRamAddr(uint16_t addr) {
	// TODO: Do another pass on the logic here
	if (addr >= 0xA000 && addr <= 0xBFFF) {
		if (ramEnable) {
			// Use the ram bank number bits if in ram mode
			uint8_t newRamBankNumber = 0;
			if (modeSelect == 0x01) {
				newRamBankNumber = ramBankNumber;
			}
			return newRamBankNumber * RAM_BANK_SIZE + (addr - 0xA000);
		}
		else {
			return 0xFFFFFFFF;
		}
	}
	else {
		// Error
		return 0xFFFFFFFF;
	}
}

void MBC1::setRegister(uint16_t addr, uint8_t data) {
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
		// Upper rom number bits or ram number bits, depending on mode
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
