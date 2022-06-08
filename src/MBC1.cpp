#include "MBC1.h"

MBC1::MBC1(uint8_t romBanks, uint8_t ramBanks)
	: MBC(romBanks, ramBanks) {}

uint32_t MBC1::mapRomAddr(uint16_t addr) {
	if (addr <= 0x3FFF) {
		uint8_t newRomBankNumber = 0;
		if (mode == 0x01) {
			newRomBankNumber = bank2 << 5;
		}
		uint32_t newAddr = newRomBankNumber * util::ROM_BANK_SIZE + addr;
		newAddr %= romBanks * util::ROM_BANK_SIZE;
		return newAddr;
	}
	else if (addr <= 0x7FFF) {
		// Use the upper rom bank number bits if in rom mode
		uint8_t newRomBankNumber = (bank2 << 5) | bank1;
		uint32_t newAddr = newRomBankNumber * util::ROM_BANK_SIZE + (addr - 0x4000);
		newAddr %= romBanks * util::ROM_BANK_SIZE;
		return newAddr;
	}
	else {
		// Error
		return 0xFFFFFFFF;
	}
}

uint32_t MBC1::mapRamAddr(uint16_t addr) {
	// TODO: Do another pass on the logic here
	if (addr >= 0xA000 && addr <= 0xBFFF) {
		if (ramg) {
			// Use the ram bank number bits if in ram mode
			uint8_t newRamBankNumber = 0;
			if (mode == 0x01) {
				newRamBankNumber = bank2;
			}
			uint32_t newAddr = newRamBankNumber * util::RAM_BANK_SIZE + (addr - 0xA000);
			newAddr %= ramBanks * util::RAM_BANK_SIZE;
			return newAddr;
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
		ramg = (data & 0x0F) == 0x0A;
	}
	else if (addr <= 0x3FFF) {
		// Rom bank number, bottom 5 bits
		data &= 0x1F;
		if (data == 0x00) {
			data = 0x01;
		}
		bank1 = data;
	}
	else if (addr <= 0x5FFF) {
		// Upper rom number bits or ram number bits, depending on mode
		data &= 0x03;
		bank2 = data;
	}
	else if (addr <= 0x7FFF) {
		data &= 0x01;
		mode = data;
	}
}
