#include "MMU.h"

#include "APU.h"

APU::APU(MMU* mmu) {
	this->mmu = mmu;
}

void APU::clock() {
	// Store output in a circular buffer with separate read and write positions.
	// Write a new value on every clock tick until the write position is the same as the read position.
	
	// TODO: Consider switching to using the frame sequencer

	updateFrameSequencer();
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

void APU::updateFrameSequencer() {
	// Reset all clocks (assume they are to be consumed, only true for 1 tick)
	// TODO: Consider moving this out of the hottest loop and only have them reset when
	// the channels consume them
	lengthCtrClock = false;
	sweepClock = false;
	volEnvClock = false;

	
	// Step the frame sequencer forward at 512 Hz
	if (fsCounter == 0) {
		fsCounter = 2048;
		
		// Length counter, step 0, 2, 4, 6
		if ((fsStep & 0x01) == 0x00) {
			lengthCtrClock = true;
		}
		
		// Sweep, step 2, 6
		if ((fsStep & 0x03) == 0x02) {
			sweepClock = true;
		}

		// Vol Env, step 7
		if ((fsStep & 0x07) == 0x07) {
			volEnvClock = true;
		}
		
		fsStep++;
	}
	fsCounter--;
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

void APU::triggerChannel1() {
	// Trigger event:
	// Channel is enabled (volume set to 0, internal enabled flag cleared)
	// If length counter is 0, set to 64
	// Freq timer reloaded with period
	// Vol env timer reloaded with period
	// Channel volume reloaded from register (NRx2)
	// Noise channel lfsr bits all set to 1
	// Wave channels position set to 0 (sample buffer not refilled)
	// Sweep
	// - copied to shadow reg
	// - sweep timer reloaded
	// - internal enabled flag set if either sweep period or shift are non zero, cleared otherwise
	// - if sweep shift is non zero, freq calculation and overflow check performed immediately

	// Read relevant memory locations
	uint8_t nr10 = mmu->directRead(0xFF10);
	uint8_t nr12 = mmu->directRead(0xFF12);
	uint8_t nr13 = mmu->directRead(0xFF13);
	uint8_t nr14 = mmu->directRead(0xFF14);

	soundOn1 = 1;
	sweepCounter1 = (nr10 & 0x70) >> 4;				// sweep period
	volume1 = (nr12 & 0xF0) >> 4;					// initial volume
	envelopeCounter1 = (nr12 & 0x07);				// envelope period

	if (soundLengthCounter1 == 0) {
		soundLengthCounter1 = 64;			// always reset to 64 according to gbdevwiki sound hardware page
	}

	uint16_t frequency = nr13;
	frequency |= (uint16_t)(nr14 & 0x07) << 8;
	frequencyCounter1 = 2048 - frequency;		// (2048 - freq) * 4 for T-cycles

	waveIndex1 = 0;
}

void APU::triggerChannel2() {
	uint8_t nr22 = mmu->directRead(0xFF17);
	uint8_t nr23 = mmu->directRead(0xFF18);
	uint8_t nr24 = mmu->directRead(0xFF19);

	soundOn2 = 1;
	volume2 = (nr22 & 0xF0) >> 4;				// initial volume
	envelopeCounter2 = (nr22 & 0x07);			// envelope period

	if (soundLengthCounter2 == 0) {
		soundLengthCounter2 = 64;
	}

	uint16_t frequency = nr23;
	frequency |= (uint16_t)(nr24 & 0x07) << 8;
	frequencyCounter2 = 2048 - frequency;

	waveIndex2 = 0;
}

void APU::triggerChannel3() {
	uint8_t nr30 = mmu->directRead(0xFF1A);
	uint8_t nr31 = mmu->directRead(0xFF1B);
	uint8_t nr32 = mmu->directRead(0xFF1C);
	uint8_t nr33 = mmu->directRead(0xFF1D);
	uint8_t nr34 = mmu->directRead(0xFF1E);

	soundOn3 = 1;
	soundLengthCounter3 = 256 - nr31;

	uint8_t volumeIndex = (nr32 & 0x60) >> 5;
	volume3 = waveVolume[volumeIndex];

	uint16_t frequency = nr33;
	frequency |= (uint16_t)(nr34 & 0x07) << 8;
	frequencyCounter3 = (2048 - frequency) >> 1;		// (2048 - freq) * 2 for T-cycles
	
	waveIndex3 = 0;

	// If DAC is off, the above actions occur but the channel is disabled again (gbdevwiki sound hardware page)
	uint8_t soundOn = (nr30 & 0x80) >> 6;
	if (soundOn == 0) {
		soundOn3 = 0;
	}
}

void APU::triggerChannel4() {
	uint8_t nr41 = mmu->directRead(0xFF20);
	uint8_t nr42 = mmu->directRead(0xFF21);
	uint8_t nr43 = mmu->directRead(0xFF22);
	uint8_t nr44 = mmu->directRead(0xFF23);

	soundOn4 = 1;
	volume4 = (nr42 & 0xF0) >> 4;				// initial volume
	envelopeCounter4 = (nr42 & 0x07);			// envelope period

	if (soundLengthCounter4 == 0) {
		soundLengthCounter4 = 64;
	}

	uint8_t shiftAmount = (nr43 & 0xF0) >> 4;
	uint8_t divisorCode = (nr43 & 0x07);
	uint8_t divisor = noiseDivisor[divisorCode];

	frequencyCounter4 = divisor << shiftAmount;

	shiftRegister = 0xFFFF;
}

void APU::updateChannel1Timer(uint8_t data) {
	soundLengthCounter1 = 64 - data;
}

void APU::updateChannel2Timer(uint8_t data) {
	soundLengthCounter2 = 64 - data;
}

void APU::updateChannel3Timer(uint8_t data) {
	soundLengthCounter3 = 256 - data;
}

void APU::updateChannel4Timer(uint8_t data) {
	soundLengthCounter4 = 64 - data;
}

void APU::updateChannel1() {
	// TODO: Could this be out of sync with what the CPU is expecting? ie the CPU sets a length and restarts within
	// the unchecked ticks, and then changes the length, expecting it to already have been consumed?
	// Sound length counter, tick once every 256 Hz or 4096 cycles
	if (lengthCtrClock) {
		uint8_t nr14 = mmu->directRead(0xFF14);
		uint8_t lengthEnabled = (nr14 & 0x40) >> 6;
		
		if (lengthEnabled) {
			soundLengthCounter1--;
		}
	}
	if (soundLengthCounter1 == 0) {
		// Don't return immediately so 0 is written to the channel 1 buffer
		soundOn1 = 0;
	}
	
	// Sweep counter, tick once every 128 Hz or 8192 cycles
	if (sweepClock) {
		sweepCounter1--;
	}
	if (sweepCounter1 == 0) {
		// Read the sweep register
		uint8_t nr10 = mmu->directRead(0xFF10);
		uint8_t sweepPeriod = (nr10 & 0x70) >> 4;
		uint8_t sweepDirection = (nr10 & 0x08) >> 3;
		uint8_t sweepShift = (nr10 & 0x07);

		sweepCounter1 = sweepPeriod;

		// Only use sweep if sweep period is greater than 0
		if (sweepPeriod > 0 && sweepShift > 0) {
			// Read the frequency
			uint8_t nr13 = mmu->directRead(0xFF13);
			uint8_t nr14 = mmu->directRead(0xFF14);
			uint16_t frequency = nr13;						// lower 8 bits
			frequency |= (uint16_t)(nr14 & 0x07) << 8;		// upper 8 bits

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

	// Envelop counter, tick once every 64 Hz or 16384 cycles
	if (volEnvClock) {
		envelopeCounter1--;
	}
	if (envelopeCounter1 == 0) {
		// Read the envelope register
		uint8_t nr12 = mmu->directRead(0xFF12);
		uint8_t envelopeDirection = (nr12 & 0x08) >> 3;
		uint8_t envelopePeriod = (nr12 & 0x07);

		envelopeCounter1 = envelopePeriod;

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

	// Frequency counter
	frequencyCounter1--;
	if (frequencyCounter1 == 0) {
		uint8_t nr13 = mmu->directRead(0xFF13);
		uint8_t nr14 = mmu->directRead(0xFF14);
		uint16_t frequency = nr13;						// lower 8 bits
		frequency |= (uint16_t)(nr14 & 0x07) << 8;		// upper 8 bits

		frequencyCounter1 = 2048 - frequency;

		// Get the correct wave pattern
		uint8_t nr11 = mmu->directRead(0xFF11);
		uint8_t wavePatternDutyIndex = (nr11 & 0xC0) >> 6;
		uint8_t waveRatio = squareWaveRatio[wavePatternDutyIndex];

		// Update the sample produced in accordance with the wave frequency
		sample1 = (waveIndex1 & 0x07) < waveRatio ? -1.0f : 1.0f;

		// Letting it just wrap around for now
		waveIndex1++;
	}
	
	// Fill the buffer every tick
	buffer1[bufferIndex1] = soundOn1 * volume1 * sample1;

	bufferIndex1++;
	if (bufferIndex1 > 1023) {
		bufferIndex1 = 0;
	}
}

void APU::updateChannel2() {
	// Length counter
	if (lengthCtrClock) {
		uint8_t nr24 = mmu->directRead(0xFF19);
		uint8_t lengthEnabled = (nr24 & 0x40) >> 6;

		if (lengthEnabled) {
			soundLengthCounter2--;
		}
	}
	if (soundLengthCounter2 == 0) {
		soundOn2 = 0;
	}

	// Volume envelope counter
	if (volEnvClock) {
		envelopeCounter2--;
	}
	if (envelopeCounter2 == 0) {
		uint8_t nr22 = mmu->directRead(0xFF17);
		uint8_t envelopePeriod = (nr22 & 0x07);
		uint8_t envelopeDirection = (nr22 & 0x08) >> 3;

		envelopeCounter2 = envelopePeriod;

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

	// Frequency counter
	frequencyCounter2--;
	if (frequencyCounter2 == 0) {
		uint8_t nr23 = mmu->directRead(0xFF18);
		uint8_t nr24 = mmu->directRead(0xFF19);
		uint16_t frequency = nr23;						// lower 8 bits
		frequency |= (uint16_t)(nr24 & 0x07) << 8;		// upper 8 bits

		frequencyCounter2 = 2048 - frequency;

		// Get the correct wave pattern
		uint8_t nr21 = mmu->directRead(0xFF16);
		uint8_t wavePatternDutyIndex = (nr21 & 0xC0) >> 6;
		uint8_t waveRatio = squareWaveRatio[wavePatternDutyIndex];

		// Update the sample produced in accordance with the wave frequency
		sample2 = (waveIndex2 & 0x07) < waveRatio ? -1.0f : 1.0f;

		waveIndex2++;
	}

	// Fill the buffer every tick
	buffer2[bufferIndex2] = soundOn2 * volume2 * sample2;

	bufferIndex2++;
	if (bufferIndex2 > 1023) {
		bufferIndex2 = 0;
	}
}

void APU::updateChannel3() {
	// Length counter
	if (lengthCtrClock) {
		uint8_t nr34 = mmu->directRead(0xFF1E);
		uint8_t lengthEnabled = (nr34 & 0x40) >> 6;

		if (lengthEnabled) {
			soundLengthCounter3--;
		}
	}
	if (soundLengthCounter3 == 0) {
		soundOn3 = 0;
	}

	// Frequency counter
	frequencyCounter3--;
	if (frequencyCounter3 == 0) {
		uint8_t nr33 = mmu->directRead(0xFF1D);
		uint8_t nr34 = mmu->directRead(0xFF1E);
		uint16_t frequency = nr33;
		frequency |= (uint16_t)(nr34 & 0x07) << 8;

		frequencyCounter3 = (2048 - frequency) >> 1;

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

	// Fill the buffer every tick
	buffer3[bufferIndex3] = soundOn3 * volume3 * sample3;

	bufferIndex3++;
	if (bufferIndex3 > 1023) {
		bufferIndex3 = 0;
	}
}

void APU::updateChannel4() {
	// Length counter
	if (lengthCtrClock) {
		uint8_t nr44 = mmu->directRead(0xFF23);
		uint8_t lengthEnabled = (nr44 & 0x40) >> 6;

		if (lengthEnabled) {
			soundLengthCounter4--;
		}
	}
	if (soundLengthCounter4 == 0) {
		soundOn4 = 0;
	}

	// Volume envelope counter
	if (volEnvClock) {
		envelopeCounter4--;
	}
	if (envelopeCounter4 == 0) {
		uint8_t nr42 = mmu->directRead(0xFF21);
		uint8_t envelopePeriod = (nr42 & 0x07);
		uint8_t envelopeDirection = (nr42 & 0x08) >> 3;

		envelopeCounter4 = envelopePeriod;

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

	// Frequency counter
	frequencyCounter4--;
	if (frequencyCounter4 == 0) {
		uint8_t nr43 = mmu->directRead(0xFF22);
		uint8_t shiftAmount = (nr43 & 0xF0) >> 4;
		uint8_t divisorCode = (nr43 & 0x07);
		uint8_t divisor = noiseDivisor[divisorCode];

		frequencyCounter4 = divisor << shiftAmount;

		// Use the inverted first bit as the sample (-1.0f to 1.0f)
		sample4 = -2.0f * (float)(shiftRegister & 0x01) + 1.0f;

		// Shift register operation
		uint16_t result = (shiftRegister & 0x01) ^ ((shiftRegister & 0x02) >> 1);
		shiftRegister >>= 1;

		shiftRegister &= ~(result << 14); 
		shiftRegister |= (result << 14);

		uint8_t widthMode = (nr43 & 0x08) >> 3;
		if (widthMode) {
			shiftRegister &= ~(1 << 6);
			shiftRegister |= (result << 6);
		}
	}
	
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
