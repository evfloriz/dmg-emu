#include "MBC3.h"

MBC3::MBC3(uint8_t romBanks, uint8_t ramBanks)
	: MBC(romBanks, ramBanks) {}

uint32_t MBC3::mapRomAddr(uint16_t addr) {
	if (addr <= 0x3FFF) {
		return addr;
	}
	else if (addr <= 0x7FFF) {
		uint32_t newAddr = bank1 * ROM_BANK_SIZE + (addr - 0x4000);
		newAddr %= romBanks * ROM_BANK_SIZE;
		return newAddr;
	}
	else {
		// Error
		return 0xFFFFFFFF;
	}
}

uint32_t MBC3::mapRamAddr(uint16_t addr) {
	if (addr >= 0xA000 && addr <= 0xBFFF) {
		if (ramg) {
			if (mode == 0x01) {
				// RTC
				return 0xFFFFFFFF;
			}
			else {
				uint32_t newAddr = bank2 * RAM_BANK_SIZE + (addr - 0xA000);
				newAddr %= ramBanks * RAM_BANK_SIZE;
				return newAddr;
			}
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

void MBC3::setRegister(uint16_t addr, uint8_t data) {
	// TODO: Implement RTC
	if (addr <= 0x1FFF) {
		// Ram enable
		data &= 0x0A;
		ramg = data;
	}
	else if (addr <= 0x3FFF) {
		// Rom bank number, bottom 7 bits
		data &= 0x7F;
		if (data == 0x00) {
			data = 0x01;
		}
		bank1 = data;
	}
	else if (addr <= 0x5FFF) {
		// Ram bank number or RTC
		// Use modeSelect to keep track of ram bank number or rtc (0 = ram, 1 = rtc)
		if (data <= 0x03) {
			bank2 = data;
			mode = 0x00;
		}
		else if (data >= 0x08 && data <= 0x0C) {
			// RTC
			mode = 0x01;
		}
	}
	else if (addr <= 0x7FFF) {
		// RTC
	}
}
