#include "MMU.h"

#include "APU.h"

APU::APU(MMU* mmu) {
	this->mmu = mmu;
}

void APU::clock() {
	// Store output in a circular buffer with separate read and write positions.
	// Write a new value on every clock tick until the write position is the same as the read position.
	
	// TODO: Consider switching to using the frame sequencer

	updateControl();
	
	updateChannel1();
	updateChannel2();
	updateChannel3();
	updateChannel4();
	
	// Resample from every tick (2^20 Hz) to sample rate (44100 Hz)
	if (sampleCounter <= 0) {
		int posDifference = getPosDifference();
		
		if (posDifference > 3072) {
			// Writing too fast, slow down
			sampleCounter += slowerTicks;
		}
		else if (posDifference < 2048) {
			// Writing too slow, speed up
			sampleCounter += fasterTicks;
		}
		else {
			sampleCounter += sampleTicks;
		}
		
		// Drop the sample if write is about to overflow into the read position
		writeCounter++;
		if (writePos == readPos) {
			writesDropped++;
			return;
		}

		// Mix samples
		float channels[4] = {
			buffer1[bufferIndex1],
			buffer2[bufferIndex2],
			buffer3[bufferIndex3],
			buffer4[bufferIndex4]
		};
		
		float mixSO2 = 0.0f;
		float mixSO1 = 0.0f;
		for (int i = 0; i < 4; i++) {
			mixSO2 += channels[i] * selectionSO2[i];
			mixSO1 += channels[i] * selectionSO1[i];
		}

		// TODO: Make this an option in the options text file
		/*int channel = 1;
		mixSO2 = channels[channel - 1] * selectionSO2[channel - 1];
		mixSO1 = channels[channel - 1] * selectionSO1[channel - 1];*/

		outputSO2[writePos] = volume * volumeSO2 * mixSO2;
		outputSO1[writePos] = volume * volumeSO1 * mixSO1;

		// Wrap around to 0 if writePos exceeds the size
		writePos++;
		if (writePos > size - 1) {
			writePos = 0;
		}

	}
	sampleCounter--;
}

int APU::getPosDifference() {
	return (writePos - readPos + size) % size;
}

void APU::fillBuffer(float* stream, int len) {
	// Set the value of the stream to the output, starting at the current read position and wrapping around
	// if the read position exceeds the size.
	for (int i = 0; i < len; i += 2) {
		stream[i] = outputSO2[readPos];
		stream[i + 1] = outputSO1[readPos];
		
		readPos++;

		readCounter++;
		if (readPos == writePos) {
			readsDropped++;
		}

		if (readPos > size - 1) {
			readPos = 0;
		}
	}
}

void APU::updateChannel1() {
	// Read relevant memory locations
	uint8_t nr10 = mmu->directRead(0xFF10);
	uint8_t nr11 = mmu->directRead(0xFF11);
	uint8_t nr12 = mmu->directRead(0xFF12);
	uint8_t nr13 = mmu->directRead(0xFF13);
	uint8_t nr14 = mmu->directRead(0xFF14);

	// Split registers into their encoded information
	uint8_t sweepPeriod = (nr10 & 0x70) >> 4;
	uint8_t sweepDirection = (nr10 & 0x08) >> 3;
	uint8_t sweepShift = (nr10 & 0x07);
	uint8_t wavePatternDutyIndex = (nr11 & 0xC0) >> 6;
	uint8_t soundLength = (nr11 & 0x3F);
	uint8_t initialVolume = (nr12 & 0xF0) >> 4;
	uint8_t envelopeDirection = (nr12 & 0x08) >> 3;
	uint8_t envelopePeriod = (nr12 & 0x07);
	uint8_t restart = (nr14 & 0x80) >> 7;
	uint8_t selection = (nr14 & 0x40) >> 6;

	uint16_t frequency = nr13;						// lower 8 bits
	frequency |= (uint16_t)(nr14 & 0x07) << 8;		// upper 8 bits

	// Restart sound
	if (restart == 1) {
		soundOn1 = 1;
		volume1 = initialVolume;

		// Reset counters
		soundLengthCounter1 = 4096 * (64 - soundLength);
		sweepCounter1 = 8192 * sweepPeriod;
		envelopeCounter1 = 16384 * envelopePeriod;

		// For now, consume the bit
		mmu->setBit(0xFF14, 7, 0);
	}

	// If sound is off, simply return
	if (soundOn1 == 0) {
		return;
	}

	// TODO: Could this be out of sync with what the CPU is expecting? ie the CPU sets a length and restarts within
	// the unchecked ticks, and then changes the length, expecting it to already have been consumed?
	// Sound length counter, tick once every 256 Hz or 4096 cycles
	if (soundLengthCounter1 == 0) {
		soundLengthCounter1 = 4096 * (64 - soundLength);

		// If selection is true, stop the sound. Otherwise keep playing
		if (selection) {
			// Don't return immediately so 0 is written to the channel 1 buffer
			soundOn1 = 0;
		}
	}
	soundLengthCounter1--;

	// Sweep counter, tick once every 128 Hz or 8192 cycles
	if (sweepCounter1 == 0) {
		sweepCounter1 = 8192 * sweepPeriod;

		// Only use sweep if sweep period is greater than 0
		if (sweepPeriod > 0 && sweepShift > 0) {
			// Increase or decrease frequency within 0 and 2047, turning off the channel if it goes out of bounds
			// 0 is an increase, 1 is a decrease
			if (sweepDirection == 0) {
				frequency = frequency + (frequency / (1 << sweepShift));
			}
			else {
				frequency = frequency - (frequency / (1 << sweepShift));
			}

			if (frequency > 2047) {
				soundOn1 = 0;
				frequency = 2047;			// prevent division by 0
			}
				
			// Write frequency back to nr13 and nr14
			nr13 = frequency & 0x00FF;
				
			nr14 &= 0xF8;
			nr14 |= (frequency & 0x0700) >> 8;

			mmu->directWrite(0xFF13, nr13);
			mmu->directWrite(0xFF14, nr14);

			// Calculate the next frequency and do the overflow check again
			uint16_t overflowCheck = 0;
			if (sweepDirection == 0) {
				overflowCheck = frequency + (frequency / (1 << sweepShift));
			}
			else {
				overflowCheck = frequency - (frequency / (1 << sweepShift));
			}

			if (overflowCheck > 2047) {
				soundOn1 = 0;
			}
		}
	}
	sweepCounter1--;
	
	// Envelop counter, tick once every 64 Hz or 16384 cycles
	if (envelopeCounter1 == 0) {
		envelopeCounter1 = 16384 * envelopePeriod;

		// Only use envelope if envelope period is greater than 0
		if (envelopePeriod > 0) {
			// Increase or decrease volume within 0x00 and 0x0F
			if (envelopeDirection == 1 && volume1 < 0x0F) {
				volume1++;
			}
			else if (envelopeDirection == 0 && volume1 > 0x00) {
				volume1--;
			}
		}
	}
	envelopeCounter1--;
	
	// Set the wave duty ratio (will be divided by 8)
	uint8_t waveRatio1 = 4;
	switch (wavePatternDutyIndex) {
	case 0:
		waveRatio1 = 1;
		break;
	case 1:
		waveRatio1 = 2;
		break;
	case 2:
		waveRatio1 = 4;
		break;
	case 3:
		waveRatio1 = 6;
		break;
	}

	uint32_t tone1 = 131072 / (2048 - frequency);				// from pandocs

	if (frequencyCounter1 == 0) {
		// Split into 8 equal parts
		frequencyCounter1 = (1 << 20) / tone1 / 8;

		// Update the sample produced in accordance with the wave frequency
		sample1 = waveIndex1 % 8 < waveRatio1 ? -1.0f : 1.0f;

		// Letting it just wrap around for now
		waveIndex1++;
	}
	frequencyCounter1--;

	// Fill the buffer every tick
	buffer1[bufferIndex1] = soundOn1 * volume1 * sample1;

	bufferIndex1++;
	if (bufferIndex1 > 1023) {
		bufferIndex1 = 0;
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
		soundOn2 = 1;
		volume2 = initialVolume;

		// Reset counters
		soundLengthCounter2 = 4096 * (64 - soundLength);
		envelopeCounter2 = 16384 * envelopePeriod;

		frequencyCounter2 = 0;

		// For now, consume the bit
		mmu->setBit(0xFF19, 7, 0);
	}
	
	// If sound is off, simply return
	if (soundOn2 == 0) {
		return;
	}

	// Sound length counter, tick once every 256 Hz or 4096 cycles
	if (soundLengthCounter2 == 0) {
		soundLengthCounter2 = 4096 * (64 - soundLength);
		
		// If selection is true, stop the sound. Otherwise keep playing
		if (selection) {
			soundOn2 = 0;
		}
	}
	soundLengthCounter2--;

	// Envelop counter, tick once every 64 Hz or 16384 cycles
	if (envelopeCounter2 == 0) {
		envelopeCounter2 = 16384 * envelopePeriod;

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
	envelopeCounter2--;

	// Set the wave duty ratio (will be divided by 8)
	uint8_t waveRatio2 = 4;
	switch (wavePatternDutyIndex) {
	case 0:
		waveRatio2 = 1;
		break;
	case 1:
		waveRatio2 = 2;
		break;
	case 2:
		waveRatio2 = 4;
		break;
	case 3:
		waveRatio2 = 6;
		break;
	}

	uint32_t tone2 = 131072 / (2048 - frequency);				// from pandocs

	if (frequencyCounter2 == 0) {
		// Split into 8 equal parts
		frequencyCounter2 = (1 << 20) / tone2 / 8;

		// Update the sample produced in accordance with the wave frequency
		sample2 = waveIndex2 % 8 < waveRatio2 ? -1.0f : 1.0f;

		// Letting it just wrap around for now
		waveIndex2++;
	}
	frequencyCounter2--;

	// Fill the buffer every tick
	buffer2[bufferIndex2] = soundOn2 * volume2 * sample2;

	bufferIndex2++;
	if (bufferIndex2 > 1023) {
		bufferIndex2 = 0;
	}
}

void APU::updateChannel3() {
	// Read relevant memory locations
	uint8_t nr30 = mmu->directRead(0xFF1A);
	uint8_t nr31 = mmu->directRead(0xFF1B);
	uint8_t nr32 = mmu->directRead(0xFF1C);
	uint8_t nr33 = mmu->directRead(0xFF1D);
	uint8_t nr34 = mmu->directRead(0xFF1E);

	// Split registers into their encoded information
	uint8_t soundOn = (nr30 & 0x80) >> 6;
	uint8_t soundLength = nr31;
	uint8_t volume = (nr32 & 0x60) >> 5;
	uint8_t restart = (nr34 & 0x80) >> 7;
	uint8_t selection = (nr34 & 0x40) >> 6;

	uint16_t frequency = nr33;						// lower 8 bits
	frequency |= (uint16_t)(nr34 & 0x07) << 8;		// upper 8 bits

	// Return right away if sound is off
	if (soundOn == 0) {
		soundOn3 = 0;

		// TODO: Investigate if this results in not filling the buffer with 0s, and if that
		// causes issues
		return;
	}

	// Restart sound
	if (restart == 1) {	
		soundOn3 = 1;
		switch (volume) {
		case 0:
			volume3 = 0;
			break;
		case 1:
			volume3 = 0x0F;		// 100%
			break;
		case 2:
			volume3 = 0x07;		// 50%
			break;
		case 3:
			volume3 = 0x03;		// 25%
			break;
		}

		// Reset counters
		soundLengthCounter3 = 4096 * (256 - soundLength);

		// For now, consume the bit
		mmu->setBit(0xFF1E, 7, 0);
	}

	// If sound is off, simply return
	if (soundOn3 == 0) {
		return;
	}

	// Sound length counter, tick once every 256 Hz or 4096 cycles
	if (soundLengthCounter3 == 0) {
		soundLengthCounter3 = 4096 * (256 - soundLength);

		// If selection is true, stop the sound. Otherwise keep playing
		if (selection) {
			soundOn3 = 0;
		}
	}
	soundLengthCounter3--;

	uint32_t tone3 = 65536 / (2048 - frequency);

	if (frequencyCounter3 == 0) {
		// Split into 32 portions
		frequencyCounter3 = (1 << 20) / tone3 / 32;
		if (frequencyCounter3 == 0) {
			frequencyCounter3 = 1;
		}

		uint8_t sampleByte = mmu->directRead(0xFF30 + (waveIndex3 / 2));

		// If it's even, use the upper 4 bits, otherwise read the lower 4 bits
		if (waveIndex3 % 2 == 0) {
			sampleByte = (sampleByte & 0xF0) >> 4;
		}
		else {
			sampleByte &= 0x0F;
		}

		// Convert the byte into a float from -1.0 to 1.0
		sample3 = 2.0f * float(sampleByte) / 0xF - 1.0f;

		// Every tick of the frequency counter, read in the next sample
		waveIndex3++;
		if (waveIndex3 > 31) {
			waveIndex3 = 0;
		}
	}
	frequencyCounter3--;

	// Fill the buffer every tick
	buffer3[bufferIndex3] = soundOn3 * volume3 * sample3;

	bufferIndex3++;
	if (bufferIndex3 > 1023) {
		bufferIndex3 = 0;
	}
}

void APU::updateChannel4() {
	// Read relevant memory locations
	uint8_t nr41 = mmu->directRead(0xFF20);
	uint8_t nr42 = mmu->directRead(0xFF21);
	uint8_t nr43 = mmu->directRead(0xFF22);
	uint8_t nr44 = mmu->directRead(0xFF23);

	// Split registers into their encoded information
	uint8_t soundLength = (nr41 & 0x3F);
	uint8_t initialVolume = (nr42 & 0xF0) >> 4;
	uint8_t envelopeDirection = (nr42 & 0x08) >> 3;
	uint8_t envelopePeriod = (nr42 & 0x07);
	uint8_t restart = (nr44 & 0x80) >> 7;
	uint8_t selection = (nr44 & 0x40) >> 6;

	uint8_t shiftClockFrequency = (nr43 & 0xF0) >> 4;
	uint8_t widthMode = (nr43 & 0x08) >> 3;
	uint8_t dividingRatio = (nr43 & 0x07);

	// Restart sound
	if (restart == 1) {
		soundOn4 = 1;
		volume4 = initialVolume;

		// Reset counters
		soundLengthCounter4 = 4096 * (64 - soundLength);
		envelopeCounter4 = 16384 * envelopePeriod;

		// For now, consume the bit
		mmu->setBit(0xFF23, 7, 0);
	}

	// If sound is off, simply return
	if (soundOn4 == 0) {
		return;
	}

	// Sound length counter, tick once every 256 Hz or 4096 cycles
	if (soundLengthCounter4 == 0) {
		soundLengthCounter4 = 4096 * (64 - soundLength);

		// If selection is true, stop the sound. Otherwise keep playing
		if (selection) {
			soundOn4 = 0;
		}
	}
	soundLengthCounter4--;

	// Envelop counter, tick once every 64 Hz or 16384 cycles
	if (envelopeCounter4 == 0) {
		envelopeCounter4 = 16384 * envelopePeriod;

		// Only use envelope if envelope period is greater than 0
		if (envelopePeriod > 0) {
			// Increase or decrease volume within 0x00 and 0x0F
			if (envelopeDirection == 1 && volume4 < 0x0F) {
				volume4++;
			}
			else if (envelopeDirection == 0 && volume4 > 0x00) {
				volume4--;
			}
		}
	}
	envelopeCounter4--;

	// If dividing ratio is 0, treat it like 0.5
	uint32_t dividend = (dividingRatio > 0) ? (524288 / dividingRatio) : (524288 * 2);
	uint32_t tone4 = dividend >> (shiftClockFrequency + 1);
	
	if (frequencyCounter4 == 0) {
		frequencyCounter4 = (1 << 20) / tone4;
		
		uint8_t noiseBit = shiftRegister & 0x01;

		// Use the inverted noise bit as the sample
		sample4 = noiseBit ? -1.0f : 1.0f;

		// Shift register operation
		uint16_t result = (shiftRegister & 0x01) ^ ((shiftRegister & 0x02) >> 1);
		shiftRegister >>= 1;

		shiftRegister &= ~(result << 14); 
		shiftRegister |= (result << 14);

		if (widthMode) {
			shiftRegister &= ~(1 << 6);
			shiftRegister |= (result << 6);
		}
	}
	frequencyCounter4--;

	// Fill the buffer every tick
	buffer4[bufferIndex4] = soundOn4 * volume4 * sample4;

	bufferIndex4++;
	if (bufferIndex4 > 1023) {
		bufferIndex4 = 0;
	}	
}

void APU::toggleSound(uint8_t data) {
	// TODO: Destroy all contents of sound registers when sound is turned off
	soundOn = data;
}

void APU::updateControl() {
	uint8_t nr50 = mmu->directRead(0xFF24);
	uint8_t nr51 = mmu->directRead(0xFF25);

	volumeSO2 = (nr50 & 0x70) >> 4;
	volumeSO1 = (nr50 & 0x07);

	selectionSO2[3] = (nr51 & 0x80) >> 7;
	selectionSO2[2] = (nr51 & 0x40) >> 6;
	selectionSO2[1] = (nr51 & 0x20) >> 5;
	selectionSO2[0] = (nr51 & 0x10) >> 4;

	selectionSO1[3] = (nr51 & 0x08) >> 3;
	selectionSO1[2] = (nr51 & 0x04) >> 2;
	selectionSO1[1] = (nr51 & 0x02) >> 1;
	selectionSO1[0] = (nr51 & 0x01);
}
