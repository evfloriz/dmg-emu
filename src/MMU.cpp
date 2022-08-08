#include "CPU.h"
#include "PPU.h"
#include "APU.h"

#include "MMU.h"

MMU::MMU(CPU* cpu, PPU* ppu, APU* apu) {
	this->cpu = cpu;
	this->ppu = ppu;
	this->apu = apu;

	// Set joypad register to high for now
	memory[0xFF00] = 0xFF;
}

MMU::~MMU() {
	delete[] memory;
}

void MMU::write(uint16_t addr, uint8_t data) {
	if (addr <= 0x7FFF) {
		// Change the state of mbc chip if there is one
		cart->setRegister(addr, data);
		return;
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		// External ram
		cart->writeRam(addr, data);
		return;
	}
	else if ((addr >= 0xE000 && addr <= 0xFDFF) ||		// Echo ram, prohibited
			(addr >= 0xFEA0 && addr <= 0xFEFF)) {		// Unusable	
		return;
	}
	
	switch (addr) {
	case 0xFF00: {
		// Change which set of buttons is selected to be read from
		if (~data & (1 << 5)) {
			selectedButtons = &actionButtons;
		}
		else if (~data & (1 << 4)) {
			selectedButtons = &directionButtons;
		}
		else {
			// Set selected buttons to off, always 0xFF
			// TODO: Double check that this is the correct behaviour.
			selectedButtons = &buttonsOff;
		}
		break;
	}
	case 0xFF04: {
		// If the divider is written to, set it to 0
		cpu->resetDivider();
		break;
	}
	case 0xFF05: {
		// If the divider is written to, set it to 0
		cpu->timer.counter = data;
		break;
	}
	case 0xFF06: {
		// If the divider is written to, set it to 0
		cpu->timer.modulo = data;
		break;
	}
	case 0xFF07: {
		// If the divider is written to, set it to 0
		cpu->timer.on = data & 0x04;
		cpu->timer.speedIndex = data & 0x03;
		cpu->timer.speed = cpu->timerSpeeds[data & 0x03];
		break;
	}
	case 0xFF0F: {
		// IF
		cpu->IF = data;
		break;
	}
	case 0xFF11: {
		memory[addr] = data;

		// Update the channel 1 length timer
		data &= 0x3F;
		apu->channel1.lengthCounter = 64 - data;
		break;
	}
	case 0xFF12: {
		memory[addr] = data;
		apu->channel1.dacPower = (data & 0xF8);
		
		// If dac power is off, disable channel 1
		if (!apu->channel1.dacPower) {
			apu->channel1.soundOn = 0;
		}
		break;
	}
	case 0xFF14: {
		memory[addr] = data;
		// Bit 7 restarts channel 1
		if (data & 0x80) {
			apu->triggerChannel1();
		}
		break;
	}
	case 0xFF16: {
		memory[addr] = data;
		data &= 0x3F;
		apu->channel2.lengthCounter = 64 - data;
		break;
	}
	case 0xFF17: {
		memory[addr] = data;
		apu->channel2.dacPower = (data & 0xF8);
		
		// If dac power is off, disable channel 2
		if (!apu->channel2.dacPower) {
			apu->channel2.soundOn = 0;
		}
		break;
	}
	case 0xFF19: {
		memory[addr] = data;
		if (data & 0x80) {
			apu->triggerChannel2();
		}
		break;
	}
	case 0xFF1A: {
		memory[addr] = data;
		apu->channel3.dacPower = (data & 0x80);

		// If dac power is off, disable channel 3
		if (!apu->channel3.dacPower) {
			apu->channel3.soundOn = 0;
		}

		break;
	}
	case 0xFF1B: {
		memory[addr] = data;
		apu->channel3.lengthCounter = 256 - data;
		break;
	}
	case 0xFF1E: {
		memory[addr] = data;
		if (data & 0x80) {
			apu->triggerChannel3();
		}
		break;
	}
	case 0xFF20: {
		memory[addr] = data;
		data &= 0x3F;
		apu->channel4.lengthCounter = 64 - data;
		break;
	}
	case 0xFF21: {
		memory[addr] = data;
		apu->channel4.dacPower = (data & 0xF8);

		// If dac power is off, disable channel 4
		if (!apu->channel4.dacPower) {
			apu->channel4.soundOn = 0;
		}
		break;
	}
	case 0xFF22: {
		memory[addr] = data;
		uint8_t shiftAmount = (data & 0xF0) >> 4;
		uint8_t divisor = apu->noiseDivisor[(data & 0x07)];

		apu->channel4.frequencyPeriod = (divisor << shiftAmount) >> 2;
		apu->channel4.widthMode = (data & 0x08) >> 3;
		break;
	}
	case 0xFF23: {
		memory[addr] = data;
		if (data & 0x80) {
			apu->triggerChannel4();
		}
		break;
	}
	case 0xFF24: {
		memory[addr] = data;
		apu->volumeSO2 = (data & 0x70) >> 4;
		apu->volumeSO1 = (data & 0x07);
		break;
	}
	case 0xFF25: {
		memory[addr] = data;
	
		apu->channel4.so2 = (data & 0x80) >> 7;
		apu->channel3.so2 = (data & 0x40) >> 6;
		apu->channel2.so2 = (data & 0x20) >> 5;
		apu->channel1.so2 = (data & 0x10) >> 4;

		apu->channel4.so1 = (data & 0x08) >> 3;
		apu->channel3.so1 = (data & 0x04) >> 2;
		apu->channel2.so1 = (data & 0x02) >> 1;
		apu->channel1.so1 = (data & 0x01);

		break;
	}
	case 0xFF26: {
		// Bit 7 turns all sound on or off
		data &= 0x80;
		memory[addr] &= 0x7F;
		memory[addr] |= data;

		data >>= 7;
		apu->channel1.soundOn = data;
		apu->channel2.soundOn = data;
		apu->channel3.soundOn = data;
		apu->channel4.soundOn = data;

		break;
	}
	case 0xFF40: {
		// LCDC
		memory[addr] = data;
		ppu->lcdc = data;
		break;
	}
	case 0xFF41: {
		// LCD STAT
		data &= 0xF8;
		data |= (memory[addr] & 0x07);		// Bottom 3 bits are read only

		memory[addr] = data;
		ppu->stat = data;
		break;
	}
	case 0xFF44: {
		// LY
		memory[addr] = data;
		ppu->ly = data;
		break;
	}
	case 0xFF46: {
		// DMA transfer
		// Early out if data is out of range
		if (data > 0xDF) {
			return;
		}

		// Write data from 0xXX00-0xXX9F to 0xFE00-0xFE9F
		uint16_t dataStart = data * 0x100;
		uint16_t oamStart = 0xFE00;
		for (int i = 0; i < 0xA0; i++) {
			directWrite(oamStart + i, directRead(dataStart + i));
		}
		break;
	}
	case 0xFFFF: {
		// IE
		cpu->IE = data;
		break;
	}
	default:
		memory[addr] = data;
	}
}

uint8_t MMU::read(uint16_t addr) {	
	if (addr <= 0x7FFF) {
		// Cartridge
		return cart->readRom(addr);
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		// External ram
		return cart->readRam(addr);
	}
	else if ((addr >= 0xE000 && addr <= 0xFDFF) ||		// Echo ram, prohibited
		(addr >= 0xFEA0 && addr <= 0xFEFF)) {			// Unusable	
		return 0xFF;
	}

	switch (addr) {
	case 0xFF00: {
		// Return the int that selectedButtons currently points to based on the previous write
		return *selectedButtons;
	}
	case 0xFF04: {
		return cpu->timer.divider;
	}
	case 0xFF05: {
		return cpu->timer.counter;
	}
	case 0xFF06: {
		return cpu->timer.modulo;
	}
	case 0xFF07: {
		return (cpu->timer.on << 2) | cpu->timer.speedIndex;
	}
	case 0xFF0F: {
		return cpu->IF;
	}
	case 0xFF40: {
		// LCDC
		return ppu->lcdc;
	}
	case 0xFF41: {
		// STAT
		return ppu->stat;
	}
	case 0xFF44: {
		// LY
		return ppu->ly;
	}
	case 0xFFFF: {
		return cpu->IE;
	}
	default: 
		return memory[addr];
	}
	
}

void MMU::directWrite(uint16_t addr, uint8_t data) {
	memory[addr] = data;
}

uint8_t MMU::directRead(uint16_t addr) {
	return memory[addr];
}

void MMU::insertCartridge(const std::shared_ptr<Cartridge>& cartridge) {
	this->cart = cartridge;
}

void MMU::writeActionButton(uint8_t pos, uint8_t value) {
	writeButton(&actionButtons, pos, value);
}

void MMU::writeDirectionButton(uint8_t pos, uint8_t value) {
	writeButton(&directionButtons, pos, value);
}

void MMU::writeButton(uint8_t* buttons, uint8_t pos, uint8_t value) {
	// Create mask for the bit at pos, then or with the value at pos
	uint8_t oldButtons = *buttons;
	*buttons &= ~(1 << pos);
	*buttons |= (value << pos);

	// If a bit went from high to low, request a joypad interrupt
	if (oldButtons > *buttons) {
		setIF(4, 1);
	}
}

void MMU::setBit(uint16_t addr, uint8_t pos, uint8_t value) {
	uint8_t data = memory[addr];
	data &= ~(1 << pos);
	data |= (value << pos);
	memory[addr] = data;
}

void MMU::setIF(uint8_t pos, uint8_t value) {
	cpu->IF &= ~(1 << pos);
	cpu->IF |= (value << pos);
}
