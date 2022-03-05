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
	std::shared_ptr<Cartridge> cart;

	bool init() {
		// Passing 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, instr_timing
		size_t test_num = 2;
		
		// Get rom name
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
			"11-op a,(hl).gb",
			"instr_timing.gb"
		};
		std::string romName = "test-roms/" + test_roms[test_num];

		romName = "roms/tetris.gb";

		// Create cartridge
		cart = std::make_shared<Cartridge>(romName);
		bus.insertCartridge(cart);

		std::cout << "Beginning execution of " << romName << std::endl;

		bus.cpu.print_toggle = false;
		bus.cpu.log_toggle = false;
		bus.cpu.log_file = "log/l" + std::to_string(test_num) + ".txt";

		// Initialize output file
		if (bus.cpu.log_toggle) {
			bus.cpu.file = fopen(bus.cpu.log_file.c_str(), "w");
		}

		// Reset LY
		bus.write(0xFF44, 0x00);
		
		// Reset divider and timer registers
		bus.write(0xFF04, 0xAB);
		bus.write(0xFF05, 0x00);
		bus.write(0xFF06, 0x00);
		bus.write(0xFF07, 0xF8);

		return true;
	}

	bool tick() {
		do {
			bus.clock();
		} while (!bus.cpu.complete());

		return true;
	}
};

class Demo : public olc::PixelGameEngine {
public:
	DMG dmg;
	Bus& bus = dmg.bus;

	float residual_time = 0.0f;
	
	Demo() {
		sAppName = "DMG";
	}

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

		// Implement timer
		if (residual_time > 0.0f) {
			residual_time -= fElapsedTime;
		}
		else {
			residual_time += (1.0f / 60.0f) - fElapsedTime;
			do {
				dmg.tick();
			} while (!dmg.bus.ppu.frame_complete);
			
			dmg.bus.ppu.frame_complete = false;
		}

		// Update TileData for testing once per frame
		dmg.bus.ppu.updateTileData();
		dmg.bus.ppu.updateTileMap();

		int x = 160;
		int y = 144;
		int scale = 2;

		DrawSprite(x * scale, y * scale - 192, dmg.bus.ppu.getTileData(0), 1);
		DrawSprite(x * scale, y * scale - 128, dmg.bus.ppu.getTileData(1), 1);
		DrawSprite(x * scale, y * scale - 64, dmg.bus.ppu.getTileData(2), 1);
		
		// TODO: figure out a better scroll method
		DrawPartialSprite(0, 0, dmg.bus.ppu.getTileMap(0), dmg.bus.ppu.getSCX(), dmg.bus.ppu.getSCY(), x, y, scale);

		return true;
	}
};

int main() {
	bool graphics = true;
	
	if (graphics) {
		Demo demo;
		demo.Construct(680, 480, 2, 2);
		//demo.Construct(320, 288, 2, 2);
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
		} while (true);
	}

    return 0;
}
