
#include <iostream>

#include "MMU.h"

#include "APU.h"

APU::APU(MMU* mmu) {
	this->mmu = mmu;
}

void APU::clock() {
	// Store output in a circular buffer with separate read and write positions.
	// Write a new value on every clock tick until the write position is the same as the read position.
	
	// TODO: Consider switching to using the frame sequencer
	
	updateChannel1();
	updateChannel2();
	updateChannel3();
	updateChannel4();

	//printf("sample ticks: %f\n", sampleTicks);
	//printf("sample counter: %f\n", sampleCounter);

	// Need this after since I'll need keep things ticking I think	
	
	// Resample from every tick (2^20 Hz) to sample rate (44100 Hz)
	if (sampleCounter <= 0) {
		sampleCounter += sampleTicks;
		//sampleCounter = sampleTicks;

		if (writePos == readPos) {
			return;
		}

		// Generate sample
		float sample1 = buffer1[bufferIndex1];
		float sample2 = buffer2[bufferIndex2];
		float sample3 = buffer3[bufferIndex3];
		float sample4 = buffer4[bufferIndex4];

		//noiseBitSample = sampleWave4[bufferIndex];
		channel1 = (float)soundOn1 * volume1 * sample1;
		channel2 = (float)soundOn2 * volume2 * sample2;
		channel3 = (float)soundOn3 * volume3 * sample3;
		channel4 = (float)soundOn4 * volume4 * sample4;


		//output[writePos] = volume * channel1;
		output[writePos] = volume * (channel1 + channel2 + channel3 + channel4);

		// Wrap around to 0 if writePos exceeds the size
		writePos++;
		if (writePos >= size) {
			writePos = 0;
		}

	}
	sampleCounter--;

	

	/*if (writePos == readPos) {
		return;
	}*/

	

	/*float channel1 = 0.0f;
	if (tone1 != 0) {
		uint32_t squareWavePeriod1 = 44100 / tone1;
		uint8_t waveIndex1 = float(sampleIndex1) / (float)squareWavePeriod1 * 8;
		int sample1 = (waveIndex1 < waveRatio1) ? 1 : -1;

		channel1 = (float)soundOn1 * volume1 * sample1;

		sampleIndex1++;
		if (sampleIndex1 > squareWavePeriod1 - 1) {
			sampleIndex1 = 0;
		}
	}*/

	/*float channel2 = 0.0f;
	if (tone2 != 0) {
		uint32_t squareWavePeriod2 = 44100 / tone2;
		uint8_t waveIndex2 = float(sampleIndex2) / (float)squareWavePeriod2 * 8;
		int sample2 = (waveIndex2 < waveRatio2) ? 1 : -1;

		channel2 = (float)soundOn2 * volume2 * sample2;

		sampleIndex2++;
		if (sampleIndex2 > squareWavePeriod2 - 1) {
			sampleIndex2 = 0;
		}
	}*/
	
	/*float channel3 = 0.0f;
	if (tone3 != 0) {
		uint32_t wavePeriod3 = 44100 / tone3;
		uint8_t waveIndex3 = (float)sampleIndex3 / (float)wavePeriod3 * 32;
		float sample3 = sampleWave3[waveIndex3];

		channel3 = (float)soundOn3 * volume3 * sample3;

		sampleIndex3++;
		if (sampleIndex3 > wavePeriod3 - 1) {
			sampleIndex3 = 0;
		}
	}*/
	

	// Noise is has a clock cycle period of either 127 or 32767 (7 or 15 bit)
	// 31 or 8192 M-cycles
	// Sampling period is therefore either 32768 Hz or 32 Hz

	// 2^20 / 44100 is 23.777, which means I would need to generate a sample every 23.777 ticks
	// 2^19 is max freq, lets work with that
	// 2^19 / 44100 would be the average jump between samples, ie how many audio outputs occur between samples
	// 44100 / 2^19 would be how many samples are taken per audio output (< 1)
	// for decimals, I would have to instead of subtracting by 0 subtract by the integer portion

	
	
	/*float channel4 = 0.0f;
	if (tone4 != 0) {
		float wavePeriod4 = 44100.0f / (float)tone4 * 128.0f;
		uint8_t waveIndex4 = (float)sampleIndex4 / (float)wavePeriod4 * 128;
		//float sample4 = sampleWave4[waveIndex4];
		float sample4 = lowPassFilter(waveIndex4);

		channel4 = (float)soundOn4 * volume4 * sample4;

		sampleIndex4++;
		if (sampleIndex4 > wavePeriod4 - 1) {
			sampleIndex4 = 0;
		}
	}

	
	if (tone4 < 20000) {
		printf("frequency: %d ", tone4);
		printf("channel4: %f\n", channel4);
	}*/

	//output[writePos] = volume * (channel1 + channel2 + channel3 + channel4);
	//output[writePos] = volume * (channel1 + channel2 + channel3);
	/*output[writePos] = volume * channel2

	// Wrap around to 0 if writePos exceeds the size
	writePos++;
	if (writePos >= size) {
		writePos = 0;
	}*/
}

float APU::lowPassFilter(uint16_t index) {
	// Calculate the correct output for noise channel
	double filter[57] = {
		0.000208,
		0.000297,
		0.000338,
		0.000281,
		0.000072,
		-0.000337,
		-0.000983,
		-0.001870,
		-0.002969,
		-0.004198,
		-0.005425,
		-0.006467,
		-0.007096,
		-0.007058,
		-0.006092,
		-0.003961,
		-0.000477,
		0.004468,
		0.010879,
		0.018644,
		0.027523,
		0.037160,
		0.047099,
		0.056816,
		0.065756,
		0.073380,
		0.079209,
		0.082867,
		0.084114,
		0.082867,
		0.079209,
		0.073380,
		0.065756,
		0.056816,
		0.047099,
		0.037160,
		0.027523,
		0.018644,
		0.010879,
		0.004468,
		-0.000477,
		-0.003961,
		-0.006092,
		-0.007058,
		-0.007096,
		-0.006467,
		-0.005425,
		-0.004198,
		-0.002969,
		-0.001870,
		-0.000983,
		-0.000337,
		0.000072,
		0.000281,
		0.000338,
		0.000297,
		0.000208
	};

	double sum = 0.0f;

	uint16_t lowBound = (index - 28 + 128) % 128;
	for (uint16_t i = 0; i < 57; i++) {
		sum += filter[i] * sampleWave4[(lowBound + i) % 128];
	}

	return (float)sum;
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

void APU::updateChannel1() {
	// Read relevant memory locations
	uint8_t nr10 = mmu->directRead(0xFF10);
	uint8_t nr11 = mmu->directRead(0xFF11);
	uint8_t nr12 = mmu->directRead(0xFF12);
	uint8_t nr13 = mmu->directRead(0xFF13);
	uint8_t nr14 = mmu->directRead(0xFF14);

	// Split registers into their encoded information
	uint8_t sweepPeriod = (nr10 & 0x60) >> 4;
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
			soundOn1 = 0;
		}
	}
	soundLengthCounter1--;

	// Sweep counter, tick once every 128 Hz or 8192 cycles
	if (sweepCounter1 == 0) {
		sweepCounter1 = 8192 * sweepPeriod;

		// Only use sweep if sweep period is greater than 0
		if (sweepPeriod > 0) {
			// Increase or decrease frequency within 0 and 2047, turning off the channel if it goes out of bounds
			if (sweepDirection == 1) {
				if (frequency < 2048) {
					frequency = frequency + (frequency / (1 << sweepShift));
				}
				else {
					soundOn1 = 0;
				}
			}
			else if (sweepDirection == 0) {
				if (frequency > 0) {
					frequency = frequency - (frequency / (1 << sweepShift));
				}
				else {
					soundOn1 = 0;
				}
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
	case 4:
		waveRatio1 = 6;
		break;
	}

	tone1 = 131072 / (2048 - frequency);				// from pandocs

	if (frequencyCounter1 == 0) {
		// Split into 8 equal parts
		frequencyCounter1 = (1 << 20) / tone1 / 8.0f;

		// Update the sample produced in accordance with the wave frequency
		sample1 = waveIndex1 % 8 < waveRatio1 ? -1.0f : 1.0f;

		// Letting it just wrap around for now
		waveIndex1++;
	}
	frequencyCounter1--;

	// Fill the buffer every tick
	buffer1[bufferIndex1] = sample1;

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

	// TODO: Could this be out of sync with what the CPU is expecting? ie the CPU sets a length and restarts within
	// the unchecked ticks, and then changes the length, expecting it to already have been consumed?
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
	case 4:
		waveRatio2 = 6;
		break;
	}

	tone2 = 131072 / (2048 - frequency);				// from pandocs

	if (frequencyCounter2 == 0) {
		// Split into 8 equal parts
		frequencyCounter2 = (1 << 20) / tone2 / 8.0f;

		// Update the sample produced in accordance with the wave frequency
		sample2 = waveIndex2 % 8 < waveRatio2 ? -1.0f : 1.0f;

		// Letting it just wrap around for now
		waveIndex2++;
	}
	frequencyCounter2--;

	// Fill the buffer every tick
	buffer2[bufferIndex2] = sample2;

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

	// Turn frequency into Hz to use for sampling in main loop
	tone3 = 65536 / (2048 - frequency);

	// Frequency is every 65536 / (2048 - x) Hz or 16 * (2048 - x) ticks
	if (frequencyCounter3 == 0) {
		// Convert frequency to cycle counter
		//frequencyCounter3 = 16 * (2048 - frequency);
		//frequencyCounter3 = (2048 - frequency) / 2;
		// Split into 32 portions
		frequencyCounter3 = (1 << 20) / tone3 / 32.0f;
		if (frequencyCounter3 == 0) {
			frequencyCounter3 = 1;
		}

		// TODO: How often should I update the wave data? It probably doesn't need to happen very frequently
		//frequencyCounter3 = 32;

		// I think I only need to read in the wave whenever a sample needs to be produced
		sampleByte = mmu->directRead(0xFF30 + (waveSampleIndex3 / 2));

		// If it's even, use the upper 4 bits, otherwise read the lower 4 bits
		if (waveSampleIndex3 % 2 == 0) {
			sampleByte = (sampleByte & 0xF0) >> 4;
		}
		else {
			sampleByte &= 0x0F;
		}

		// Convert the byte into a float from -1.0 to 1.0
		sample3 = 2.0f * float(sampleByte) / 0xF - 1.0f;

		// Every tick of the frequency counter, consume the next sample
		waveSampleIndex3++;
		if (waveSampleIndex3 > 31) {
			waveSampleIndex3 = 0;
		}
	}
	frequencyCounter3--;

	// Fill the buffer every tick
	buffer3[bufferIndex3] = sample3;

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
	float dividingRatio = (nr43 & 0x07);

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
	// TODO: Could this be out of sync with what the CPU is expecting? ie the CPU sets a length and restarts within
	// the unchecked ticks, and then changes the length, expecting it to already have been consumed?
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

	if (dividingRatio == 0) {
		dividingRatio = 0.5;
	}

	tone4 = 524288 / dividingRatio / (1 << (shiftClockFrequency + 1));
	
	if (frequencyCounter4 == 0) {
		// Convert frequency to cycle counter
		frequencyCounter4 = (1 << 20) / tone4;
		
		noiseBit = shiftRegister & 0x01;

		// Invert the noise bit and put it in the array
		sample4 = noiseBit ? -1.0f : 1.0f;

		// Shift register function
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
	buffer4[bufferIndex4] = sample4;

	bufferIndex4++;
	if (bufferIndex4 > 1023) {
		bufferIndex4 = 0;
	}
	
}
