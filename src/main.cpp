/*
	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018-2019 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//#ifdef _WIN32
//#define _CRT_SECURE_NO_WARNINGS
//#endif

#define OLC_PGE_APPLICATION

#include <iostream>

#include "Bus.h"
#include "CPU.h"
#include "olcPixelGameEngine.h"

class DMG {
public:
	Bus bus;

	bool init() {
		// passing 1, 3, 4, 5, 6, 7, 8, 9, 10, 11
		size_t test_num = 2;
		
		// Load boot rom
		unsigned char memory[0x10000];
		std::string test_roms[] = {
			"cpu_instrs.gb",
			"01-special.gb",
			"02-interrupts.gb",
			"03-op sp,hl.gb",
			"04-op r,imm.gb",
			"05-op rp.gb",
			"06-ld r,r.gb",
			"07-jr,jp,call,ret,rst.gb",
			"08-misc instrs.gb",
			"09-op r,r.gb",
			"10-bit ops.gb",
			"11-op a,(hl).gb"
		};
		std::string rom = "test-roms/" + test_roms[test_num];
		//const char* rom = "DMG_ROM.bin";
		FILE* file = fopen(rom.c_str(), "rb");
		int pos = 0;
		while (fread(&memory[pos], 1, 1, file)) {
			pos++;
		}
		fclose(file);

		// Copy bootrom into memory
		pos = 0;
		for (auto m : memory) {
			bus.write(pos, m);
			pos++;
		}

		/*std::vector<uint8_t> program;
		program = {
			0x31,		// ld sp, fffe
			0xfe,
			0xff,
		};
		
		// copy test program into bootrom
		pos = 0;
		for (auto p : program) {
			bus.write(pos, p);
			pos++;
		}*/

		std::cout << "Beginning execution of " << rom << std::endl;

		bus.cpu.print_toggle = true;
		bus.cpu.log_toggle = true;
		bus.cpu.log_file = "log/l" + std::to_string(test_num) + ".txt";

		// file output
		if (bus.cpu.log_toggle) {
			bus.cpu.file = fopen(bus.cpu.log_file.c_str(), "w");
		}

		// reset LY
		bus.write(0xFF44, 0x00);

		return true;
	}

	bool tick() {
		do {
			bus.cpu.clock();
			
			// Increment LY to simulate vblank
			bus.cpu.simLY();

		} while (!bus.cpu.complete());

		return true;
	}
};

class Demo : public olc::PixelGameEngine {
public:
	DMG dmg;
	Bus& bus = dmg.bus;
	
	Demo() {
		sAppName = "dmg cpu demonstration";
	}

	// don't worry about disassembly
	//std::map<uint16_t, std::string> mapAsm;

	std::string hex(uint32_t n, uint8_t d) {
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4) {
			s[i] = "0123456789ABCDEF"[n & 0xF];
		}
		return s;
	}

	void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nColumns) {
		int nRamX = x;
		int nRamY = y;

		for (int row = 0; row < nRows; row++) {
			std::string sOffset = "$" + hex(nAddr, 4) + ":";
			for (int col = 0; col < nColumns; col++) {
				sOffset += " " + hex(bus.read(nAddr), 2);
				nAddr++;
			}
			DrawString(nRamX, nRamY, sOffset);
			nRamY += 10;
		}
	}

	void DrawCPU(int x, int y) {
		std::string status = "STATUS: ";
		DrawString(x, y, "STATUS:", olc::WHITE);

		DrawString(x + 64, y, "Z", bus.cpu.f & CPU::Z ? olc::GREEN : olc::RED);
		DrawString(x + 80, y, "N", bus.cpu.f & CPU::N ? olc::GREEN : olc::RED);
		DrawString(x + 96, y, "H", bus.cpu.f & CPU::H ? olc::GREEN : olc::RED);
		DrawString(x + 112, y, "C", bus.cpu.f & CPU::C ? olc::GREEN : olc::RED);

		DrawString(x, y + 10, "PC: $" + hex(bus.cpu.pc, 4));
		DrawString(x, y + 20, "A: $" + hex(bus.cpu.a, 2) + " [" + std::to_string(bus.cpu.a) + "]");
		DrawString(x, y + 30, "F: $" + hex(bus.cpu.f, 2) + " [" + std::to_string(bus.cpu.f) + "]");
		DrawString(x, y + 40, "B: $" + hex(bus.cpu.b, 2) + " [" + std::to_string(bus.cpu.b) + "]");
		DrawString(x, y + 50, "C: $" + hex(bus.cpu.c, 2) + " [" + std::to_string(bus.cpu.c) + "]");
		DrawString(x, y + 60, "D: $" + hex(bus.cpu.d, 2) + " [" + std::to_string(bus.cpu.d) + "]");
		DrawString(x, y + 70, "E: $" + hex(bus.cpu.e, 2) + " [" + std::to_string(bus.cpu.e) + "]");
		DrawString(x, y + 80, "H: $" + hex(bus.cpu.h, 2) + " [" + std::to_string(bus.cpu.h) + "]");
		DrawString(x, y + 90, "L: $" + hex(bus.cpu.l, 2) + " [" + std::to_string(bus.cpu.l) + "]");
		DrawString(x, y + 100, "SP: $" + hex(bus.cpu.sp, 4));
	}

	bool OnUserCreate() {
		dmg.init();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) {
		Clear(olc::DARK_BLUE);

		if (GetKey(olc::Key::SPACE).bPressed) {
			dmg.tick();
		}
		//dmg.tick();

		// what exactly should I draw for the bootrom? 0x00-0xFF I think which is the bootrom itself, then
		// whatever parts are actually used
		DrawRam(2, 2, 0x0000, 16, 16);
		DrawRam(2, 182, 0xFF00, 16, 16);
		DrawCPU(448, 2);

		return true;
	}
};

int main() {
	bool graphics = false;
	
	if (graphics) {
		Demo demo;
		demo.Construct(680, 480, 2, 2);
		demo.Start();
	}
	else {
		DMG dmg;
		dmg.init();
		int i = 0;
		int max = 1000000;
		int count = 0;
		do {
			dmg.tick();
			
			/*i++;
			if (i == max) {
				std::cout << count << " " << max << " instructions executed" << std::endl;
				count++;
				i = 0;
			}*/
			
		} while (true);
	}

    return 0;
}