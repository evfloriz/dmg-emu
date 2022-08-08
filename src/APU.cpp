#include "MMU.h"

#include "APU.h"

APU::APU(MMU* mmu) {
	this->mmu = mmu;
}

void APU::clock() {
	// Store output in a circular buffer with separate read and write positions.
	// Write a new value on every clock tick until the write position is the same as the read position.

	updateFrameSequencer();
	
	if (channel1.soundOn) {
		updateChannel1();
	}
	if (channel2.soundOn) {
		updateChannel2();
	}
	if (channel3.soundOn) {
		updateChannel3();
	}
	if (channel4.soundOn) {
		updateChannel4();
	}
	
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
		/*Channel channels[4] = {
			channel1,
			channel2,
			channel3,
			channel4
			//channel1.soundOn ? channel1.bufferSample : 0,
			//channel2.soundOn ? channel2.bufferSample : 0,
			//channel3.soundOn ? channel3.bufferSample : 0,
			//channel4.soundOn ? channel4.bufferSample : 0,
		};
		
		float mixSO2 = 0;
		float mixSO1 = 0;
		for (int i = 0; i < 4; i++) {
			mixSO2 += channels[i].soundOn * channels[i].so2 * channels[i].bufferSample;
			mixSO1 += channels[i].soundOn * channels[i].so1 * channels[i].bufferSample;
		}*/

		// TODO: Find a more elegant way to mix these channels
		float mixSO2 = channel1.bufferSample * channel1.so2 * channel1.soundOn +
			channel2.bufferSample * channel2.so2 * channel2.soundOn +
			channel3.bufferSample * channel3.so2 * channel3.soundOn +
			channel4.bufferSample * channel4.so2 * channel4.soundOn;
		
		float mixSO1 = channel1.bufferSample * channel1.so1 * channel1.soundOn +
			channel2.bufferSample * channel2.so1 * channel2.soundOn +
			channel3.bufferSample * channel3.so1 * channel3.soundOn +
			channel4.bufferSample * channel4.so1 * channel4.soundOn;

		// Convert uint8_t to float (0..16 = -1.0f..1.0f)
		mixSO2 = 2.0f * mixSO2 / 16.0f - 1.0f;
		mixSO1 = 2.0f * mixSO1 / 16.0f - 1.0f;

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
	uint8_t nr10 = mmu->directRead(0xFF10);
	uint8_t nr12 = mmu->directRead(0xFF12);
	uint8_t nr13 = mmu->directRead(0xFF13);
	uint8_t nr14 = mmu->directRead(0xFF14);

	channel1.soundOn = 1;
	channel1.sweepCounter = (nr10 & 0x70) >> 4;				// sweep period
	channel1.volume = (nr12 & 0xF0) >> 4;					// initial volume
	channel1.envelopeCounter = (nr12 & 0x07);				// envelope period

	if (channel1.lengthCounter == 0) {
		channel1.lengthCounter = 64;			// always reset to 64 according to gbdevwiki sound hardware page
	}

	uint16_t frequency = nr13;
	frequency |= (uint16_t)(nr14 & 0x07) << 8;
	channel1.frequencyCounter = 2048 - frequency;		// (2048 - freq) * 4 for T-cycles

	// TODO: Evaluate if resetting this should occur
	channel1.waveIndex = 0;

	// If DAC is off, the above actions occur but the channel is disabled again (gbdevwiki sound hardware page)
	// This is already done in the updateChannel functions this should be redundant
	if (channel1.dacPower == 0) {
		channel1.soundOn = 0;
	}
}

void APU::triggerChannel2() {
	uint8_t nr22 = mmu->directRead(0xFF17);
	uint8_t nr23 = mmu->directRead(0xFF18);
	uint8_t nr24 = mmu->directRead(0xFF19);

	channel2.soundOn = 1;
	channel2.volume = (nr22 & 0xF0) >> 4;				// initial volume
	channel2.envelopeCounter = (nr22 & 0x07);			// envelope period

	if (channel2.lengthCounter == 0) {
		channel2.lengthCounter = 64;
	}

	uint16_t frequency = nr23;
	frequency |= (uint16_t)(nr24 & 0x07) << 8;
	channel2.frequencyCounter = 2048 - frequency;

	channel2.waveIndex = 0;

	if (channel2.dacPower == 0) {
		channel2.soundOn = 0;
	}
}

void APU::triggerChannel3() {
	uint8_t nr30 = mmu->directRead(0xFF1A);
	uint8_t nr32 = mmu->directRead(0xFF1C);
	uint8_t nr33 = mmu->directRead(0xFF1D);
	uint8_t nr34 = mmu->directRead(0xFF1E);

	channel3.soundOn = 1;
	if (channel3.lengthCounter == 0) {
		channel3.lengthCounter = 256;
	}

	uint8_t volumeIndex = (nr32 & 0x60) >> 5;
	channel3.volume = waveVolume[volumeIndex];

	uint16_t frequency = nr33;
	frequency |= (uint16_t)(nr34 & 0x07) << 8;
	channel3.frequencyCounter = (2048 - frequency) >> 1;		// (2048 - freq) * 2 for T-cycles
	
	channel3.waveIndex = 0;

	if (channel3.dacPower == 0) {
		channel3.soundOn = 0;
	}
}

void APU::triggerChannel4() {
	uint8_t nr42 = mmu->directRead(0xFF21);
	uint8_t nr43 = mmu->directRead(0xFF22);
	uint8_t nr44 = mmu->directRead(0xFF23);

	channel4.soundOn = 1;
	channel4.volume = (nr42 & 0xF0) >> 4;				// initial volume
	channel4.envelopeCounter = (nr42 & 0x07);			// envelope period

	if (channel4.lengthCounter == 0) {
		channel4.lengthCounter = 64;
	}

	uint8_t shiftAmount = (nr43 & 0xF0) >> 4;
	uint8_t divisorCode = (nr43 & 0x07);

	// TODO: Profile to see if one way is faster
	uint8_t divisor = noiseDivisor[divisorCode];
	//uint8_t divisor = (divisorCode > 0) ? (divisorCode << 4) : 8;

	channel4.frequencyCounter = (divisor << shiftAmount) >> 2;			// divide by 4 for M-cycles

	shiftRegister = 0xFFFF;

	if (channel4.dacPower == 0) {
		channel4.soundOn = 0;
	}
}

void APU::updateChannel1() {
	/*if (channel1.dacPower == 0) {
		channel1.soundOn = 0;
		return;
	}*/

	// Sound length counter, tick once every 256 Hz or 4096 cycles
	if (lengthCtrClock) {
		uint8_t nr14 = mmu->directRead(0xFF14);
		uint8_t lengthEnabled = (nr14 & 0x40) >> 6;
		
		if (lengthEnabled && channel1.lengthCounter > 0) {
			channel1.lengthCounter--;
		}

		if (channel1.lengthCounter == 0) {
			channel1.soundOn = 0;
			return;
		}
	}
	
	// Sweep counter, tick once every 128 Hz or 8192 cycles
	if (sweepClock) {
		channel1.sweepCounter--;

		if (channel1.sweepCounter == 0) {
			uint8_t nr10 = mmu->directRead(0xFF10);
			uint8_t sweepPeriod = (nr10 & 0x70) >> 4;
			uint8_t sweepDirection = (nr10 & 0x08) >> 3;
			uint8_t sweepShift = (nr10 & 0x07);

			channel1.sweepCounter = sweepPeriod;

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
					channel1.soundOn = 0;
					//frequency = 2047;			// prevent division by 0
					return;
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
					channel1.soundOn = 0;
					return;
				}
			}
		}
	}
	
	// Envelop counter, tick once every 64 Hz or 16384 cycles
	if (volEnvClock) {
		channel1.envelopeCounter--;

		if (channel1.envelopeCounter == 0) {
			// Read the envelope register
			uint8_t nr12 = mmu->directRead(0xFF12);
			uint8_t envelopeDirection = (nr12 & 0x08) >> 3;
			uint8_t envelopePeriod = (nr12 & 0x07);

			channel1.envelopeCounter = envelopePeriod;

			// Only use envelope if envelope period is greater than 0
			if (envelopePeriod > 0) {
				// Increase or decrease volume within 0x00 and 0x0F
				if (envelopeDirection == 1 && channel1.volume < 0x0F) {
					channel1.volume++;
				}
				else if (envelopeDirection == 0 && channel1.volume > 0x00) {
					channel1.volume--;
				}
			}
		}
	}
	
	// Frequency counter
	channel1.frequencyCounter--;
	if (channel1.frequencyCounter == 0) {
		uint8_t nr13 = mmu->directRead(0xFF13);
		uint8_t nr14 = mmu->directRead(0xFF14);
		uint16_t frequency = nr13;						// lower 8 bits
		frequency |= (uint16_t)(nr14 & 0x07) << 8;		// upper 8 bits

		channel1.frequencyCounter = 2048 - frequency;

		// Get the correct wave pattern
		uint8_t nr11 = mmu->directRead(0xFF11);
		uint8_t wavePatternDutyIndex = (nr11 & 0xC0) >> 6;
		uint8_t waveRatio = squareWaveRatio[wavePatternDutyIndex];

		// Update the sample produced in accordance with the wave frequency
		channel1.sample = (channel1.waveIndex & 0x07) < waveRatio ? 0 : 16;

		// Letting it just wrap around for now
		channel1.waveIndex++;
	}
	
	// Fill the buffer every tick
	channel1.bufferSample = channel1.volume * channel1.sample;
	//channel1.bufferSample = channel1.soundOn * channel1.volume * channel1.sample;
	/*channel1.buffer[channel1.bufferIndex] = channel1.soundOn * channel1.volume * channel1.sample;

	channel1.bufferIndex++;
	if (channel1.bufferIndex > 1023) {
		channel1.bufferIndex = 0;
	}*/
}

void APU::updateChannel2() {
	/*if (channel2.dacPower == 0) {
		channel2.soundOn = 0;
		return;
	}*/
	
	// Length counter
	if (lengthCtrClock) {
		uint8_t nr24 = mmu->directRead(0xFF19);
		uint8_t lengthEnabled = (nr24 & 0x40) >> 6;

		if (lengthEnabled && channel2.lengthCounter > 0) {
			channel2.lengthCounter--;
		}

		if (channel2.lengthCounter == 0) {
			channel2.soundOn = 0;
			return;
		}
	}

	// Volume envelope counter
	if (volEnvClock) {
		channel2.envelopeCounter--;

		if (channel2.envelopeCounter == 0) {
			uint8_t nr22 = mmu->directRead(0xFF17);
			uint8_t envelopePeriod = (nr22 & 0x07);
			uint8_t envelopeDirection = (nr22 & 0x08) >> 3;

			channel2.envelopeCounter = envelopePeriod;

			// Only use envelope if envelope period is greater than 0
			if (envelopePeriod > 0) {
				// Increase or decrease volume within 0x00 and 0x0F
				if (envelopeDirection == 1 && channel2.volume < 0x0F) {
					channel2.volume++;
				}
				else if (envelopeDirection == 0 && channel2.volume > 0x00) {
					channel2.volume--;
				}
			}
		}
	}

	// Frequency counter
	channel2.frequencyCounter--;
	if (channel2.frequencyCounter == 0) {
		uint8_t nr23 = mmu->directRead(0xFF18);
		uint8_t nr24 = mmu->directRead(0xFF19);
		uint16_t frequency = nr23;						// lower 8 bits
		frequency |= (uint16_t)(nr24 & 0x07) << 8;		// upper 8 bits

		channel2.frequencyCounter = 2048 - frequency;

		// Get the correct wave pattern
		uint8_t nr21 = mmu->directRead(0xFF16);
		uint8_t wavePatternDutyIndex = (nr21 & 0xC0) >> 6;
		uint8_t waveRatio = squareWaveRatio[wavePatternDutyIndex];

		// Update the sample produced in accordance with the wave frequency
		channel2.sample = (channel2.waveIndex & 0x07) < waveRatio ? 0 : 16;

		channel2.waveIndex++;
	}

	// Fill the buffer every tick
	channel2.bufferSample = channel2.volume * channel2.sample;
	//channel2.bufferSample = channel2.soundOn * channel2.volume * channel2.sample;
	/*channel2.buffer[channel2.bufferIndex] = channel2.soundOn * channel2.volume * channel2.sample;

	channel2.bufferIndex++;
	if (channel2.bufferIndex > 1023) {
		channel2.bufferIndex = 0;
	}*/
}

void APU::updateChannel3() {
	/*if (channel3.dacPower == 0) {
		channel3.soundOn = 0;
		return;
	}*/

	// Length counter
	if (lengthCtrClock) {
		uint8_t nr34 = mmu->directRead(0xFF1E);
		uint8_t lengthEnabled = (nr34 & 0x40) >> 6;

		if (lengthEnabled && channel3.lengthCounter > 0) {
			channel3.lengthCounter--;
		}

		if (channel3.lengthCounter == 0) {
			channel3.soundOn = 0;
			return;
		}
	}

	// Frequency counter
	channel3.frequencyCounter--;
	if (channel3.frequencyCounter == 0) {
		uint8_t nr33 = mmu->directRead(0xFF1D);
		uint8_t nr34 = mmu->directRead(0xFF1E);
		uint16_t frequency = nr33;
		frequency |= (uint16_t)(nr34 & 0x07) << 8;

		channel3.frequencyCounter = (2048 - frequency) >> 1;

		uint8_t sampleByte = mmu->directRead(0xFF30 + (channel3.waveIndex / 2));

		// If it's even, use the upper 4 bits, otherwise read the lower 4 bits
		if ((channel3.waveIndex & 0x01) == 0) {
			sampleByte = (sampleByte & 0xF0) >> 4;
		}
		else {
			sampleByte &= 0x0F;
		}

		// Convert the byte into a float from -1.0 to 1.0
		channel3.sample = sampleByte;

		// Every tick of the frequency counter, read in the next sample
		channel3.waveIndex++;
		if (channel3.waveIndex > 31) {
			channel3.waveIndex = 0;
		}
	}

	// Fill the buffer every tick
	channel3.bufferSample = channel3.volume * channel3.sample;
	//channel3.bufferSample = channel3.soundOn * channel3.volume * channel3.sample;
	/*channel3.buffer[channel3.bufferIndex] = channel3.soundOn * channel3.volume * channel3.sample;

	channel3.bufferIndex++;
	if (channel3.bufferIndex > 1023) {
		channel3.bufferIndex = 0;
	}*/
}

void APU::updateChannel4() {
	/*if (channel4.dacPower == 0) {
		channel4.soundOn = 0;
		return;
	}*/

	// Length counter
	if (lengthCtrClock) {
		uint8_t nr44 = mmu->directRead(0xFF23);
		uint8_t lengthEnabled = (nr44 & 0x40) >> 6;

		if (lengthEnabled && channel4.lengthCounter > 0) {
			channel4.lengthCounter--;
		}

		if (channel4.lengthCounter == 0) {
			channel4.soundOn = 0;
			return;
		}
	}

	// Volume envelope counter
	if (volEnvClock) {
		channel4.envelopeCounter--;

		if (channel4.envelopeCounter == 0) {
			uint8_t nr42 = mmu->directRead(0xFF21);
			uint8_t envelopePeriod = (nr42 & 0x07);
			uint8_t envelopeDirection = (nr42 & 0x08) >> 3;

			channel4.envelopeCounter = envelopePeriod;

			// Only use envelope if envelope period is greater than 0
			if (envelopePeriod > 0) {
				// Increase or decrease volume within 0x00 and 0x0F
				if (envelopeDirection == 1 && channel4.volume < 0x0F) {
					channel4.volume++;
				}
				else if (envelopeDirection == 0 && channel4.volume > 0x00) {
					channel4.volume--;
				}
			}
		}
	}

	// Frequency counter
	channel4.frequencyCounter--;
	if (channel4.frequencyCounter == 0) {
		//uint8_t nr43 = mmu->directRead(0xFF22);
		//uint8_t shiftAmount = (nr43 & 0xF0) >> 4;
		//uint8_t divisorCode = (nr43 & 0x07);

		// TODO: Profile to see if one way is faster
		//uint8_t divisor = noiseDivisor[channel4.divisorCode];
		//uint8_t divisor = (divisorCode > 0) ? (divisorCode << 4) : 8;

		//channel4.frequencyCounter = (channel4.divisor << channel4.shiftAmount) >> 2;
		channel4.frequencyCounter = channel4.frequencyPeriod;

		// Use the inverted first bit as the sample (-1.0f to 1.0f)
		//channel4.sample = -2.0f * (float)(shiftRegister & 0x01) + 1.0f;
		channel4.sample = ((shiftRegister & 0x01) ^ 0x01) << 4;

		// Shift register operation
		uint16_t result = (shiftRegister & 0x01) ^ ((shiftRegister & 0x02) >> 1);
		shiftRegister >>= 1;

		shiftRegister &= ~(result << 14); 
		shiftRegister |= (result << 14);

		//uint8_t widthMode = (nr43 & 0x08) >> 3;
		if (channel4.widthMode) {
			shiftRegister &= ~(1 << 6);
			shiftRegister |= (result << 6);
		}
	}
	
	// Fill the buffer every tick
	//channel4.bufferSample = channel4.soundOn * channel4.volume * channel4.sample;
	channel4.bufferSample = channel4.volume * channel4.sample;
	/*channel4.buffer[channel4.bufferIndex] = channel4.soundOn * channel4.volume * channel4.sample;
	
	channel4.bufferIndex++;
	if (channel4.bufferIndex > 1023) {
		channel4.bufferIndex = 0;
	}*/	
}

void APU::toggleSound(uint8_t data) {
	// TODO: Destroy all contents of sound registers when sound is turned off
	soundOn = data;
}
