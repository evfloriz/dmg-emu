
#include <iostream>

#include "MMU.h"

#include "APU.h"

APU::APU(MMU* mmu) {
	this->mmu = mmu;
}

void APU::clock() {
	// Store output in a circular buffer with separate read and write positions.
	// Write a new value on every clock tick until the write position is the same as the read position.
	updateChannel2();
	
	if (writePos == readPos) {
		return;
	}

	output[writePos] = volume * soundOn * volume2 * (sin(sinIndex) > wavePatternDuty[waveIndex] ? 1 : -1);
		
	sinIndex += tone * 3.14f * 2 / 44100;
	if (sinIndex >= 3.14f * 2) {
		sinIndex -= 3.14f * 2;
	}

	// Wrap around to 0 if writePos exceeds the size
	writePos++;
	if (writePos >= size) {
		writePos = 0;
	}
}

void APU::fillBuffer(float* stream, int len) {
	// Set the value of the stream to the output, starting at the current read position and wrapping around
	// if the read position exceeds the size.
	for (int i = 0; i < len; i++) {
		stream[i] = output[readPos];
		readPos++;
		if (readPos >= size) {
			readPos = 0;
		}
	}
}

void APU::updateChannel2() {
	// Read relevant memory locations
	uint8_t nr21 = mmu->directRead(0xFF16);
	uint8_t nr22 = mmu->directRead(0xFF17);
	uint8_t nr23 = mmu->directRead(0xFF18);
	uint8_t nr24 = mmu->directRead(0xFF19);

	// Split registers into their encoded information
	uint8_t wavePatternDutyIndex	= (nr21 & 0xC0) >> 6;
	uint8_t soundLength				= (nr21 & 0x3F);
	uint8_t initialVolume			= (nr22 & 0xF0) >> 4;
	uint8_t envelopeDirection		= (nr22 & 0x08) >> 3;
	uint8_t envelopePeriod			= (nr22 & 0x07);
	uint8_t restart					= (nr24 & 0x80) >> 7;
	uint8_t selection				= (nr24 & 0x40) >> 6;
	
	uint16_t frequency = nr23;						// lower 8 bits
	frequency |= (uint16_t)(nr24 & 0x07) << 8;		// upper 8 bits

	// Restart sound
	if (restart == 1) {
		soundOn = 1;
		volume2 = initialVolume;

		// Reset counters
		soundLengthCounter = 4096 * (64 - soundLength);
		envelopeCounter = 16384 * envelopePeriod;

		// For now, consume the bit
		mmu->setBit(0xFF19, 7, 0);
	}
	
	// If sound is off, simply return
	if (soundOn == 0) {
		return;
	}

	tone = (float)131072 / (float)(2048 - frequency);				// from pandocs
	waveIndex = wavePatternDutyIndex;

	// TODO: Could this be out of sync with what the CPU is expecting? ie the CPU sets a length and restarts within
	// the unchecked ticks, and then changes the length, expecting it to already have been consumed?
	// Sound length counter, tick once every 256 Hz or 4096 cycles
	if (soundLengthCounter == 0) {
		soundLengthCounter = 4096 * (64 - soundLength);
		
		// If selection is true, stop the sound. Otherwise keep playing
		if (selection) {
			soundOn = 0;
		}
	}
	soundLengthCounter--;

	// Envelop counter, tick once every 64 Hz or 16384 cycles
	if (envelopeCounter == 0) {
		envelopeCounter = 16384 * envelopePeriod;

		// Only use envelope if envelope period is greater than 0
		if (envelopePeriod > 0) {
			// Increase or decrease volume within 0x00 and 0x0F
			if (envelopeDirection == 1 && volume2 < 0x0F) {
				volume2++;
			}
			else if (envelopeDirection == 0 && volume2 > 0x00) {
				volume2--;
			}
		}
	}
	envelopeCounter--;

	
}
