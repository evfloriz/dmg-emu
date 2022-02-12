#include "CPU.h"

#include "Bus.h"
#include <iostream>

CPU::CPU() {

	lookup = {
		{0x00, {&CPU::NOP,			1}},					{0x08, {&CPU::LD_a16_SP,	5}},
		{0x01, {&CPU::LD_r16,		3,		&b, &c}},		{0x09, {&CPU::ADD_r16,		2,		&b, &c}},
		{0x02, {&CPU::LD_p16_r8,	2,		&b, &c, &a}},	{0x0A, {&CPU::LD_r8_p16,	2,		&a, &b, &c}},
		{0x03, {&CPU::INC_r16,		2,		&b, &c}},		{0x0B, {&CPU::DEC_r16,		2,		&a, &b}},
		{0x04, {&CPU::INC,			1,		&b}},			{0x0C, {&CPU::INC,			1,		&c}},
		{0x05, {&CPU::DEC,			1,		&b}},			{0x0D, {&CPU::DEC,			1,		&c}},
		{0x06, {&CPU::LD_r8_r8,		2,		&b}},			{0x0E, {&CPU::LD_r8_r8,		2,		&c}},
		{0x07, {&CPU::RLCA,			1}},					{0x0F, {&CPU::RRCA,			1}},
		
		{0x10, {&CPU::STOP,			1}},					{0x18, {&CPU::JR,			2,		&i_N}},
		{0x11, {&CPU::LD_r16,		3,		&d, &e}},		{0x19, {&CPU::ADD_r16,		2,		&d, &e}},
		{0x12, {&CPU::LD_p16_r8,	2,		&d, &e, &a}},	{0x1A, {&CPU::LD_r8_p16,	2,		&a, &d, &e}},
		{0x13, {&CPU::INC_r16,		2,		&d, &e}},		{0x1B, {&CPU::DEC_r16,		2,		&a, &c}},
		{0x14, {&CPU::INC,			1,		&d}},			{0x1C, {&CPU::INC,			1,		&e}},
		{0x15, {&CPU::DEC,			1,		&d}},			{0x1D, {&CPU::DEC,			1,		&e}},
		{0x16, {&CPU::LD_r8_r8,		2,		&d}},			{0x1E, {&CPU::LD_r8_r8,		2,		&e}},
		{0x17, {&CPU::RLA,			1}},					{0x1F, {&CPU::RRA,			1}},
				
		{0x20, {&CPU::JR,			2,		&i_NZ}},		{0x28, {&CPU::JR,			2,		&i_Z}},
		{0x21, {&CPU::LD_r16,		3,		&h, &l}},		{0x29, {&CPU::ADD_r16,		2,		&h, &l}},
		{0x22, {&CPU::LD_HLI_A,		2,		&h, &l, &a}},	{0x2A, {&CPU::LD_A_HLI,		2,		&a, &h, &l}},
		{0x23, {&CPU::INC_r16,		2,		&h, &l}},		{0x2B, {&CPU::DEC_r16,		2,		&a, &d}},
		{0x24, {&CPU::INC,			1,		&h}},			{0x2C, {&CPU::INC,			1,		&l}},
		{0x25, {&CPU::DEC,			1,		&h}},			{0x2D, {&CPU::DEC,			1,		&l}},
		{0x26, {&CPU::LD_r8_r8,		2,		&h}},			{0x2E, {&CPU::LD_r8_r8,		2,		&l}},
		{0x27, {&CPU::DAA,			1}},					{0x2F, {&CPU::CPL,			1}},
		
		{0x30, {&CPU::JR,			2,		&i_NC}},		{0x38, {&CPU::JR,			2,		&i_C}},
		{0x31, {&CPU::LD_SP,		3}},					{0x39, {&CPU::ADD_SP,		2}},
		{0x32, {&CPU::LD_HLD_A,		2,		&h, &l, &a}},	{0x3A, {&CPU::LD_A_HLD,		2,		&a, &h, &l}},
		{0x33, {&CPU::INC_SP,		2}},					{0x3B, {&CPU::DEC_SP,		2}},
		{0x34, {&CPU::INC,			3,		&h, &l}},		{0x3C, {&CPU::INC,			1,		&a}},
		{0x35, {&CPU::DEC,			3,		&h, &l}},		{0x3D, {&CPU::DEC,			1,		&a}},
		{0x36, {&CPU::LD_p16_r8,	3,		&h, &l}},		{0x3E, {&CPU::LD_r8_r8,		2,		&a}},
		{0x37, {&CPU::SCF,			1}},					{0x3F, {&CPU::CCF,			1}},

		{0x40, {&CPU::LD_r8_r8,		1,		&b, &b}},		{0x48, {&CPU::LD_r8_r8,		1,		&c, &b}},
		{0x41, {&CPU::LD_r8_r8,		1,		&b, &c}},		{0x49, {&CPU::LD_r8_r8,		1,		&c, &c}},
		{0x42, {&CPU::LD_r8_r8,		1,		&b, &d}},		{0x4A, {&CPU::LD_r8_r8,		1,		&c, &d}},
		{0x43, {&CPU::LD_r8_r8,		1,		&b, &e}},		{0x4B, {&CPU::LD_r8_r8,		1,		&c, &e}},
		{0x44, {&CPU::LD_r8_r8,		1,		&b,	&h}},		{0x4C, {&CPU::LD_r8_r8,		1,		&c,	&h}},
		{0x45, {&CPU::LD_r8_r8,		1,		&b, &l}},		{0x4D, {&CPU::LD_r8_r8,		1,		&c, &l}},
		{0x46, {&CPU::LD_r8_p16,	2,		&b, &h, &l}},	{0x4E, {&CPU::LD_r8_p16,	2,		&c, &h, &l}},
		{0x47, {&CPU::LD_r8_r8,		1,		&b, &a}},		{0x4F, {&CPU::LD_r8_r8,		1,		&c, &a}},

		{0x50, {&CPU::LD_r8_r8,		1,		&d, &b}},		{0x58, {&CPU::LD_r8_r8,		1,		&e, &b}},
		{0x51, {&CPU::LD_r8_r8,		1,		&d, &c}},		{0x59, {&CPU::LD_r8_r8,		1,		&e, &c}},
		{0x52, {&CPU::LD_r8_r8,		1,		&d, &d}},		{0x5A, {&CPU::LD_r8_r8,		1,		&e, &d}},
		{0x53, {&CPU::LD_r8_r8,		1,		&d, &e}},		{0x5B, {&CPU::LD_r8_r8,		1,		&e, &e}},
		{0x54, {&CPU::LD_r8_r8,		1,		&d,	&h}},		{0x5C, {&CPU::LD_r8_r8,		1,		&e,	&h}},
		{0x55, {&CPU::LD_r8_r8,		1,		&d, &l}},		{0x5D, {&CPU::LD_r8_r8,		1,		&e, &l}},
		{0x56, {&CPU::LD_r8_p16,	2,		&d, &h, &l}},	{0x5E, {&CPU::LD_r8_p16,	2,		&e, &h, &l}},
		{0x57, {&CPU::LD_r8_r8,		1,		&d, &a}},		{0x5F, {&CPU::LD_r8_r8,		1,		&e, &a}},
		
		{0x60, {&CPU::LD_r8_r8,		1,		&h, &b}},		{0x68, {&CPU::LD_r8_r8,		1,		&l, &b}},
		{0x61, {&CPU::LD_r8_r8,		1,		&h, &c}},		{0x69, {&CPU::LD_r8_r8,		1,		&l, &c}},
		{0x62, {&CPU::LD_r8_r8,		1,		&h, &d}},		{0x6A, {&CPU::LD_r8_r8,		1,		&l, &d}},
		{0x63, {&CPU::LD_r8_r8,		1,		&h, &e}},		{0x6B, {&CPU::LD_r8_r8,		1,		&l, &e}},
		{0x64, {&CPU::LD_r8_r8,		1,		&h,	&h}},		{0x6C, {&CPU::LD_r8_r8,		1,		&l,	&h}},
		{0x65, {&CPU::LD_r8_r8,		1,		&h, &l}},		{0x6D, {&CPU::LD_r8_r8,		1,		&l, &l}},
		{0x66, {&CPU::LD_r8_p16,	2,		&h, &h, &l}},	{0x6E, {&CPU::LD_r8_p16,	2,		&l, &h, &l}},
		{0x67, {&CPU::LD_r8_r8,		1,		&h, &a}},		{0x6F, {&CPU::LD_r8_r8,		1,		&l, &a}},
		
		{0x70, {&CPU::LD_p16_r8,	2,		&h, &l, &b}},	{0x78, {&CPU::LD_r8_r8,		1,		&a, &b}},
		{0x71, {&CPU::LD_p16_r8,	2,		&h, &l, &c}},	{0x79, {&CPU::LD_r8_r8,		1,		&a, &c}},
		{0x72, {&CPU::LD_p16_r8,	2,		&h, &l, &d}},	{0x7A, {&CPU::LD_r8_r8,		1,		&a, &d}},
		{0x73, {&CPU::LD_p16_r8,	2,		&h, &l, &e}},	{0x7B, {&CPU::LD_r8_r8,		1,		&a, &e}},
		{0x74, {&CPU::LD_p16_r8,	2,		&h, &l, &h}},	{0x7C, {&CPU::LD_r8_r8,		1,		&a, &h}},
		{0x75, {&CPU::LD_p16_r8,	2,		&h, &l, &l}},	{0x7D, {&CPU::LD_r8_r8,		1,		&a, &l}},
		{0x76, {&CPU::HALT,			1}},					{0x7E, {&CPU::LD_r8_p16,	2,		&a, &h, &l}},
		{0x77, {&CPU::LD_p16_r8,	2,		&h, &l, &a}},	{0x7F, {&CPU::LD_r8_r8,		1,		&a, &a}},

		{0x80, {&CPU::ADD,			1,		&a, &b}},		{0x88, {&CPU::ADC,			1,		&a, &b}},
		{0x81, {&CPU::ADD,			1,		&a, &c}},		{0x89, {&CPU::ADC,			1,		&a, &c}},
		{0x82, {&CPU::ADD,			1,		&a, &d}},		{0x8A, {&CPU::ADC,			1,		&a, &d}},
		{0x83, {&CPU::ADD,			1,		&a, &e}},		{0x8B, {&CPU::ADC,			1,		&a, &e}},
		{0x84, {&CPU::ADD,			1,		&a, &h}},		{0x8C, {&CPU::ADC,			1,		&a, &h}},
		{0x85, {&CPU::ADD,			1,		&a, &l}},		{0x8D, {&CPU::ADC,			1,		&a, &l}},
		{0x86, {&CPU::ADD,			2,		&a, &h, &l}},	{0x8E, {&CPU::ADC,			2,		&a, &h, &l}},
		{0x87, {&CPU::ADD,			1,		&a, &a}},		{0x8F, {&CPU::ADC,			1,		&a, &a}},

		{0x90, {&CPU::SUB,			1,		&a, &b}},		{0x98, {&CPU::SBC,			1,		&a, &b}},
		{0x91, {&CPU::SUB,			1,		&a, &c}},		{0x99, {&CPU::SBC,			1,		&a, &c}},
		{0x92, {&CPU::SUB,			1,		&a, &d}},		{0x9A, {&CPU::SBC,			1,		&a, &d}},
		{0x93, {&CPU::SUB,			1,		&a, &e}},		{0x9B, {&CPU::SBC,			1,		&a, &e}},
		{0x94, {&CPU::SUB,			1,		&a, &h}},		{0x9C, {&CPU::SBC,			1,		&a, &h}},
		{0x95, {&CPU::SUB,			1,		&a, &l}},		{0x9D, {&CPU::SBC,			1,		&a, &l}},
		{0x96, {&CPU::SUB,			2,		&a, &h, &l}},	{0x9E, {&CPU::SBC,			2,		&a, &h, &l}},
		{0x97, {&CPU::SUB,			1,		&a, &a}},		{0x9F, {&CPU::SBC,			1,		&a, &a}},

		{0xA0, {&CPU::AND,			1,		&a, &b}},		{0xA8, {&CPU::XOR,			1,		&a, &b}},
		{0xA1, {&CPU::AND,			1,		&a, &c}},		{0xA9, {&CPU::XOR,			1,		&a, &c}},
		{0xA2, {&CPU::AND,			1,		&a, &d}},		{0xAA, {&CPU::XOR,			1,		&a, &d}},
		{0xA3, {&CPU::AND,			1,		&a, &e}},		{0xAB, {&CPU::XOR,			1,		&a, &e}},
		{0xA4, {&CPU::AND,			1,		&a, &h}},		{0xAC, {&CPU::XOR,			1,		&a, &h}},
		{0xA5, {&CPU::AND,			1,		&a, &l}},		{0xAD, {&CPU::XOR,			1,		&a, &l}},
		{0xA6, {&CPU::AND,			2,		&a, &h, &l}},	{0xAE, {&CPU::XOR,			2,		&a, &h, &l}},
		{0xA7, {&CPU::AND,			1,		&a, &a}},		{0xAF, {&CPU::XOR,			1,		&a, &a}},

		{0xB0, {&CPU::OR,			1,		&a, &b}},		{0xB8, {&CPU::CP,			1,		&a, &b}},
		{0xB1, {&CPU::OR,			1,		&a, &c}},		{0xB9, {&CPU::CP,			1,		&a, &c}},
		{0xB2, {&CPU::OR,			1,		&a, &d}},		{0xBA, {&CPU::CP,			1,		&a, &d}},
		{0xB3, {&CPU::OR,			1,		&a, &e}},		{0xBB, {&CPU::CP,			1,		&a, &e}},
		{0xB4, {&CPU::OR,			1,		&a, &h}},		{0xBC, {&CPU::CP,			1,		&a, &h}},
		{0xB5, {&CPU::OR,			1,		&a, &l}},		{0xBD, {&CPU::CP,			1,		&a, &l}},
		{0xB6, {&CPU::OR,			2,		&a, &h, &l}},	{0xBE, {&CPU::CP,			2,		&a, &h, &l}},
		{0xB7, {&CPU::OR,			1,		&a, &a}},		{0xBF, {&CPU::CP,			1,		&a, &a}},

		{0xC0, {&CPU::RET,			2,		&i_NZ}},		{0xC8, {&CPU::RET,			2,		&i_Z}},
		{0xC1, {&CPU::POP_r16,		3,		&b,	&c}},		{0xC9, {&CPU::RET,			2,		&i_N}},
		{0xC2, {&CPU::JP,			3,		&i_NZ}},		{0xCA, {&CPU::JP,			3,		&i_Z}},
		{0xC3, {&CPU::JP,			3,		&i_N}},			{0xCB, {&CPU::CB,			0}},
		{0xC4, {&CPU::CALL,			3,		&i_NZ}},		{0xCC, {&CPU::CALL,			3,		&i_Z}},
		{0xC5, {&CPU::PUSH_r16,		4,		&b, &c}},		{0xCD, {&CPU::CALL,			3,		&i_N}},
		{0xC6, {&CPU::ADD,			2,		&a}},			{0xCE, {&CPU::ADC,			2,		&a}},
		{0xC7, {&CPU::RST,			4,		&rst[0]}},		{0xCF, {&CPU::RST,			4,		&rst[4]}},
		
		{0xD0, {&CPU::RET,			2,		&i_NC}},		{0xD8, {&CPU::RET,			2,		&i_C}},
		{0xD1, {&CPU::POP_r16,		3,		&d, &e}},		{0xD9, {&CPU::RETI,			4,		&i_N}},				
		{0xD2, {&CPU::JP,			3,		&i_NC}},		{0xDA, {&CPU::JP,			3,		&i_C}},		
		/*{0xD3}*/											/*{0xDB}*/
		{0xD4, {&CPU::CALL,			3,		&i_NC}},		{0xDC, {&CPU::CALL,			3,		&i_C}},
		{0xD5, {&CPU::PUSH_r16,		4,		&d, &e}},		/*{0xDC}*/
		{0xD6, {&CPU::SUB,			2,		&a}},			{0xDE, {&CPU::SBC,			2,		&a}},
		{0xD7, {&CPU::RST,			4,		&rst[1]}},		{0xDF, {&CPU::RST,			4,		&rst[5]}},
		
		{0xE0, {&CPU::LDH_p8_A,		3}},					{0xE8, {&CPU::ADD_SP_o8,	4}},
		{0xE1, {&CPU::POP_r16,		3,		&h, &l}},		{0xE9, {&CPU::JP,			1,		&i_N, &h, &l}},
		{0xE2, {&CPU::LDH_p8_A,		2,		&c}},			{0xEA, {&CPU::LD_a16_A,		4}},
		/*{0xE3}*/											/*{0xEB}*/
		/*{0xE4}*/											/*{0xEC}*/
		{0xE5, {&CPU::PUSH_r16,		4,		&h, &l}},		/*{0xED}*/
		{0xE6, {&CPU::AND,			2,		&a}},			{0xEE, {&CPU::XOR,			2,		&a}},
		{0xE7, {&CPU::RST,			4,		&rst[2]}},		{0xEF, {&CPU::RST,			4,		&rst[6]}},
		
		{0xF0, {&CPU::LDH_A_p8,		3}},					{0xF8, {&CPU::LD_HL_SP_o8,	3}},
		{0xF1, {&CPU::POP_AF,		3,		&a, &f}},		{0xF9, {&CPU::LD_SP,		2,		&h, &l}},
		{0xF2, {&CPU::LDH_A_p8,		2,		&c}},			{0xFA, {&CPU::LD_A_a16,		4}},
		{0xF3, {&CPU::DI,			1}},					{0xFB, {&CPU::EI,			1}},
		/*{0xF4}*/											/*{0xFC}*/
		{0xF5, {&CPU::PUSH_AF,		4,		&a, &f}},		/*{0xFD}*/
		{0xF6, {&CPU::OR,			2,		&a}},			{0xFE, {&CPU::CP,			2,		&a}},
		{0xF7, {&CPU::RST,			4,		&rst[3]}},		{0xFF, {&CPU::RST,			4,		&rst[7]}},
	};

	cb_lookup = {
		{0x00, {&CPU::RLC,			2,		&b}},			{0x08, {&CPU::RRC,			2,		&b}},
		{0x01, {&CPU::RLC,			2,		&c}},			{0x09, {&CPU::RRC,			2,		&c}},
		{0x02, {&CPU::RLC,			2,		&d}},			{0x0A, {&CPU::RRC,			2,		&d}},
		{0x03, {&CPU::RLC,			2,		&e}},			{0x0B, {&CPU::RRC,			2,		&e}},
		{0x04, {&CPU::RLC,			2,		&h}},			{0x0C, {&CPU::RRC,			2,		&h}},
		{0x05, {&CPU::RLC,			2,		&l}},			{0x0D, {&CPU::RRC,			2,		&l}},
		{0x06, {&CPU::RLC,			4,		&h, &l}},		{0x0E, {&CPU::RRC,			4,		&h, &l}},
		{0x07, {&CPU::RLC,			2,		&a}},			{0x0F, {&CPU::RRC,			2,		&a}},

		{0x10, {&CPU::RL,			2,		&b}},			{0x18, {&CPU::RR,			2,		&b}},
		{0x11, {&CPU::RL,			2,		&c}},			{0x19, {&CPU::RR,			2,		&c}},
		{0x12, {&CPU::RL,			2,		&d}},			{0x1A, {&CPU::RR,			2,		&d}},
		{0x13, {&CPU::RL,			2,		&e}},			{0x1B, {&CPU::RR,			2,		&e}},
		{0x14, {&CPU::RL,			2,		&h}},			{0x1C, {&CPU::RR,			2,		&h}},
		{0x15, {&CPU::RL,			2,		&l}},			{0x1D, {&CPU::RR,			2,		&l}},
		{0x16, {&CPU::RL,			4,		&h, &l}},		{0x1E, {&CPU::RR,			4,		&h, &l}},
		{0x17, {&CPU::RL,			2,		&a}},			{0x1F, {&CPU::RR,			2,		&a}},

		{0x20, {&CPU::SLA,			2,		&b}},			{0x28, {&CPU::SRA,			2,		&b}},
		{0x21, {&CPU::SLA,			2,		&c}},			{0x29, {&CPU::SRA,			2,		&c}},
		{0x22, {&CPU::SLA,			2,		&d}},			{0x2A, {&CPU::SRA,			2,		&d}},
		{0x23, {&CPU::SLA,			2,		&e}},			{0x2B, {&CPU::SRA,			2,		&e}},
		{0x24, {&CPU::SLA,			2,		&h}},			{0x2C, {&CPU::SRA,			2,		&h}},
		{0x25, {&CPU::SLA,			2,		&l}},			{0x2D, {&CPU::SRA,			2,		&l}},
		{0x26, {&CPU::SLA,			4,		&h, &l}},		{0x2E, {&CPU::SRA,			4,		&h, &l}},
		{0x27, {&CPU::SLA,			2,		&a}},			{0x2F, {&CPU::SRA,			2,		&a}},

		{0x30, {&CPU::SLA,			2,		&b}},			{0x38, {&CPU::SRL,			2,		&b}},
		{0x31, {&CPU::SLA,			2,		&c}},			{0x39, {&CPU::SRL,			2,		&c}},
		{0x32, {&CPU::SLA,			2,		&d}},			{0x3A, {&CPU::SRL,			2,		&d}},
		{0x33, {&CPU::SLA,			2,		&e}},			{0x3B, {&CPU::SRL,			2,		&e}},
		{0x34, {&CPU::SLA,			2,		&h}},			{0x3C, {&CPU::SRL,			2,		&h}},
		{0x35, {&CPU::SLA,			2,		&l}},			{0x3D, {&CPU::SRL,			2,		&l}},
		{0x36, {&CPU::SLA,			4,		&h, &l}},		{0x3E, {&CPU::SRL,			4,		&h, &l}},
		{0x37, {&CPU::SLA,			2,		&a}},			{0x3F, {&CPU::SRL,			2,		&a}},

		{0x40, {&CPU::BIT,			2,	&bit[0], &b}},		{0x48, {&CPU::BIT,			2,	&bit[1], &b}},
		{0x41, {&CPU::BIT,			2,	&bit[0], &c}},		{0x49, {&CPU::BIT,			2,	&bit[1], &c}},
		{0x42, {&CPU::BIT,			2,	&bit[0], &d}},		{0x4A, {&CPU::BIT,			2,	&bit[1], &d}},
		{0x43, {&CPU::BIT,			2,	&bit[0], &e}},		{0x4B, {&CPU::BIT,			2,	&bit[1], &e}},
		{0x44, {&CPU::BIT,			2,	&bit[0], &h}},		{0x4C, {&CPU::BIT,			2,	&bit[1], &h}},
		{0x45, {&CPU::BIT,			2,	&bit[0], &l}},		{0x4D, {&CPU::BIT,			2,	&bit[1], &l}},
		{0x46, {&CPU::BIT,			3,	&bit[0], &h, &l}},	{0x4E, {&CPU::BIT,			3,	&bit[1], &h, &l}},
		{0x47, {&CPU::BIT,			2,	&bit[0], &a}},		{0x4F, {&CPU::BIT,			2,	&bit[1], &a}},

		{0x50, {&CPU::BIT,			2,	&bit[2], &b}},		{0x58, {&CPU::BIT,			2,	&bit[3], &b}},
		{0x51, {&CPU::BIT,			2,	&bit[2], &c}},		{0x59, {&CPU::BIT,			2,	&bit[3], &c}},
		{0x52, {&CPU::BIT,			2,	&bit[2], &d}},		{0x5A, {&CPU::BIT,			2,	&bit[3], &d}},
		{0x53, {&CPU::BIT,			2,	&bit[2], &e}},		{0x5B, {&CPU::BIT,			2,	&bit[3], &e}},
		{0x54, {&CPU::BIT,			2,	&bit[2], &h}},		{0x5C, {&CPU::BIT,			2,	&bit[3], &h}},
		{0x55, {&CPU::BIT,			2,	&bit[2], &l}},		{0x5D, {&CPU::BIT,			2,	&bit[3], &l}},
		{0x56, {&CPU::BIT,			3,	&bit[2], &h, &l}},	{0x5E, {&CPU::BIT,			3,	&bit[3], &h, &l}},
		{0x57, {&CPU::BIT,			2,	&bit[2], &a}},		{0x5F, {&CPU::BIT,			2,	&bit[3], &a}},

		{0x60, {&CPU::BIT,			2,	&bit[4], &b}},		{0x68, {&CPU::BIT,			2,	&bit[5], &b}},
		{0x61, {&CPU::BIT,			2,	&bit[4], &c}},		{0x69, {&CPU::BIT,			2,	&bit[5], &c}},
		{0x62, {&CPU::BIT,			2,	&bit[4], &d}},		{0x6A, {&CPU::BIT,			2,	&bit[5], &d}},
		{0x63, {&CPU::BIT,			2,	&bit[4], &e}},		{0x6B, {&CPU::BIT,			2,	&bit[5], &e}},
		{0x64, {&CPU::BIT,			2,	&bit[4], &h}},		{0x6C, {&CPU::BIT,			2,	&bit[5], &h}},
		{0x65, {&CPU::BIT,			2,	&bit[4], &l}},		{0x6D, {&CPU::BIT,			2,	&bit[5], &l}},
		{0x66, {&CPU::BIT,			3,	&bit[4], &h, &l}},	{0x6E, {&CPU::BIT,			3,	&bit[5], &h, &l}},
		{0x67, {&CPU::BIT,			2,	&bit[4], &a}},		{0x6F, {&CPU::BIT,			2,	&bit[5], &a}},

		{0x70, {&CPU::BIT,			2,	&bit[6], &b}},		{0x78, {&CPU::BIT,			2,	&bit[7], &b}},
		{0x71, {&CPU::BIT,			2,	&bit[6], &c}},		{0x79, {&CPU::BIT,			2,	&bit[7], &c}},
		{0x72, {&CPU::BIT,			2,	&bit[6], &d}},		{0x7A, {&CPU::BIT,			2,	&bit[7], &d}},
		{0x73, {&CPU::BIT,			2,	&bit[6], &e}},		{0x7B, {&CPU::BIT,			2,	&bit[7], &e}},
		{0x74, {&CPU::BIT,			2,	&bit[6], &h}},		{0x7C, {&CPU::BIT,			2,	&bit[7], &h}},
		{0x75, {&CPU::BIT,			2,	&bit[6], &l}},		{0x7D, {&CPU::BIT,			2,	&bit[7], &l}},
		{0x76, {&CPU::BIT,			3,	&bit[6], &h, &l}},	{0x7E, {&CPU::BIT,			3,	&bit[7], &h, &l}},
		{0x77, {&CPU::BIT,			2,	&bit[6], &a}},		{0x7F, {&CPU::BIT,			2,	&bit[7], &a}},

		{0x80, {&CPU::RES,			2,	&bit[0], &b}},		{0x88, {&CPU::RES,			2,	&bit[1], &b}},
		{0x81, {&CPU::RES,			2,	&bit[0], &c}},		{0x89, {&CPU::RES,			2,	&bit[1], &c}},
		{0x82, {&CPU::RES,			2,	&bit[0], &d}},		{0x8A, {&CPU::RES,			2,	&bit[1], &d}},
		{0x83, {&CPU::RES,			2,	&bit[0], &e}},		{0x8B, {&CPU::RES,			2,	&bit[1], &e}},
		{0x84, {&CPU::RES,			2,	&bit[0], &h}},		{0x8C, {&CPU::RES,			2,	&bit[1], &h}},
		{0x85, {&CPU::RES,			2,	&bit[0], &l}},		{0x8D, {&CPU::RES,			2,	&bit[1], &l}},
		{0x86, {&CPU::RES,			4,	&bit[0], &h, &l}},	{0x8E, {&CPU::RES,			4,	&bit[1], &h, &l}},
		{0x87, {&CPU::RES,			2,	&bit[0], &a}},		{0x8F, {&CPU::RES,			2,	&bit[1], &a}},

		{0x90, {&CPU::RES,			2,	&bit[2], &b}},		{0x98, {&CPU::RES,			2,	&bit[3], &b}},
		{0x91, {&CPU::RES,			2,	&bit[2], &c}},		{0x99, {&CPU::RES,			2,	&bit[3], &c}},
		{0x92, {&CPU::RES,			2,	&bit[2], &d}},		{0x9A, {&CPU::RES,			2,	&bit[3], &d}},
		{0x93, {&CPU::RES,			2,	&bit[2], &e}},		{0x9B, {&CPU::RES,			2,	&bit[3], &e}},
		{0x94, {&CPU::RES,			2,	&bit[2], &h}},		{0x9C, {&CPU::RES,			2,	&bit[3], &h}},
		{0x95, {&CPU::RES,			2,	&bit[2], &l}},		{0x9D, {&CPU::RES,			2,	&bit[3], &l}},
		{0x96, {&CPU::RES,			4,	&bit[2], &h, &l}},	{0x9E, {&CPU::RES,			4,	&bit[3], &h, &l}},
		{0x97, {&CPU::RES,			2,	&bit[2], &a}},		{0x9F, {&CPU::RES,			2,	&bit[3], &a}},

		{0xA0, {&CPU::RES,			2,	&bit[4], &b}},		{0xA8, {&CPU::RES,			2,	&bit[5], &b}},
		{0xA1, {&CPU::RES,			2,	&bit[4], &c}},		{0xA9, {&CPU::RES,			2,	&bit[5], &c}},
		{0xA2, {&CPU::RES,			2,	&bit[4], &d}},		{0xAA, {&CPU::RES,			2,	&bit[5], &d}},
		{0xA3, {&CPU::RES,			2,	&bit[4], &e}},		{0xAB, {&CPU::RES,			2,	&bit[5], &e}},
		{0xA4, {&CPU::RES,			2,	&bit[4], &h}},		{0xAC, {&CPU::RES,			2,	&bit[5], &h}},
		{0xA5, {&CPU::RES,			2,	&bit[4], &l}},		{0xAD, {&CPU::RES,			2,	&bit[5], &l}},
		{0xA6, {&CPU::RES,			4,	&bit[4], &h, &l}},	{0xAE, {&CPU::RES,			4,	&bit[5], &h, &l}},
		{0xA7, {&CPU::RES,			2,	&bit[4], &a}},		{0xAF, {&CPU::RES,			2,	&bit[5], &a}},

		{0xB0, {&CPU::RES,			2,	&bit[6], &b}},		{0xB8, {&CPU::RES,			2,	&bit[7], &b}},
		{0xB1, {&CPU::RES,			2,	&bit[6], &c}},		{0xB9, {&CPU::RES,			2,	&bit[7], &c}},
		{0xB2, {&CPU::RES,			2,	&bit[6], &d}},		{0xBA, {&CPU::RES,			2,	&bit[7], &d}},
		{0xB3, {&CPU::RES,			2,	&bit[6], &e}},		{0xBB, {&CPU::RES,			2,	&bit[7], &e}},
		{0xB4, {&CPU::RES,			2,	&bit[6], &h}},		{0xBC, {&CPU::RES,			2,	&bit[7], &h}},
		{0xB5, {&CPU::RES,			2,	&bit[6], &l}},		{0xBD, {&CPU::RES,			2,	&bit[7], &l}},
		{0xB6, {&CPU::RES,			4,	&bit[6], &h, &l}},	{0xBE, {&CPU::RES,			4,	&bit[7], &h, &l}},
		{0xB7, {&CPU::RES,			2,	&bit[6], &a}},		{0xBF, {&CPU::RES,			2,	&bit[7], &a}},

		{0xC0, {&CPU::SET,			2,	&bit[0], &b}},		{0xC8, {&CPU::SET,			2,	&bit[1], &b}},
		{0xC1, {&CPU::SET,			2,	&bit[0], &c}},		{0xC9, {&CPU::SET,			2,	&bit[1], &c}},
		{0xC2, {&CPU::SET,			2,	&bit[0], &d}},		{0xCA, {&CPU::SET,			2,	&bit[1], &d}},
		{0xC3, {&CPU::SET,			2,	&bit[0], &e}},		{0xCB, {&CPU::SET,			2,	&bit[1], &e}},
		{0xC4, {&CPU::SET,			2,	&bit[0], &h}},		{0xCC, {&CPU::SET,			2,	&bit[1], &h}},
		{0xC5, {&CPU::SET,			2,	&bit[0], &l}},		{0xCD, {&CPU::SET,			2,	&bit[1], &l}},
		{0xC6, {&CPU::SET,			4,	&bit[0], &h, &l}},	{0xCE, {&CPU::SET,			4,	&bit[1], &h, &l}},
		{0xC7, {&CPU::SET,			2,	&bit[0], &a}},		{0xCF, {&CPU::SET,			2,	&bit[1], &a}},

		{0xD0, {&CPU::SET,			2,	&bit[2], &b}},		{0xD8, {&CPU::SET,			2,	&bit[3], &b}},
		{0xD1, {&CPU::SET,			2,	&bit[2], &c}},		{0xD9, {&CPU::SET,			2,	&bit[3], &c}},
		{0xD2, {&CPU::SET,			2,	&bit[2], &d}},		{0xDA, {&CPU::SET,			2,	&bit[3], &d}},
		{0xD3, {&CPU::SET,			2,	&bit[2], &e}},		{0xDB, {&CPU::SET,			2,	&bit[3], &e}},
		{0xD4, {&CPU::SET,			2,	&bit[2], &h}},		{0xDC, {&CPU::SET,			2,	&bit[3], &h}},
		{0xD5, {&CPU::SET,			2,	&bit[2], &l}},		{0xDD, {&CPU::SET,			2,	&bit[3], &l}},
		{0xD6, {&CPU::SET,			4,	&bit[2], &h, &l}},	{0xDE, {&CPU::SET,			4,	&bit[3], &h, &l}},
		{0xD7, {&CPU::SET,			2,	&bit[2], &a}},		{0xDF, {&CPU::SET,			2,	&bit[3], &a}},

		{0xE0, {&CPU::SET,			2,	&bit[4], &b}},		{0xE8, {&CPU::SET,			2,	&bit[5], &b}},
		{0xE1, {&CPU::SET,			2,	&bit[4], &c}},		{0xE9, {&CPU::SET,			2,	&bit[5], &c}},
		{0xE2, {&CPU::SET,			2,	&bit[4], &d}},		{0xEA, {&CPU::SET,			2,	&bit[5], &d}},
		{0xE3, {&CPU::SET,			2,	&bit[4], &e}},		{0xEB, {&CPU::SET,			2,	&bit[5], &e}},
		{0xE4, {&CPU::SET,			2,	&bit[4], &h}},		{0xEC, {&CPU::SET,			2,	&bit[5], &h}},
		{0xE5, {&CPU::SET,			2,	&bit[4], &l}},		{0xED, {&CPU::SET,			2,	&bit[5], &l}},
		{0xE6, {&CPU::SET,			4,	&bit[4], &h, &l}},	{0xEE, {&CPU::SET,			4,	&bit[5], &h, &l}},
		{0xE7, {&CPU::SET,			2,	&bit[4], &a}},		{0xEF, {&CPU::SET,			2,	&bit[5], &a}},

		{0xF0, {&CPU::SET,			2,	&bit[6], &b}},		{0xF8, {&CPU::SET,			2,	&bit[7], &b}},
		{0xF1, {&CPU::SET,			2,	&bit[6], &c}},		{0xF9, {&CPU::SET,			2,	&bit[7], &c}},
		{0xF2, {&CPU::SET,			2,	&bit[6], &d}},		{0xFA, {&CPU::SET,			2,	&bit[7], &d}},
		{0xF3, {&CPU::SET,			2,	&bit[6], &e}},		{0xFB, {&CPU::SET,			2,	&bit[7], &e}},
		{0xF4, {&CPU::SET,			2,	&bit[6], &h}},		{0xFC, {&CPU::SET,			2,	&bit[7], &h}},
		{0xF5, {&CPU::SET,			2,	&bit[6], &l}},		{0xFD, {&CPU::SET,			2,	&bit[7], &l}},
		{0xF6, {&CPU::SET,			4,	&bit[6], &h, &l}},	{0xFE, {&CPU::SET,			4,	&bit[7], &h, &l}},
		{0xF7, {&CPU::SET,			2,	&bit[6], &a}},		{0xFF, {&CPU::SET,			2,	&bit[7], &a}},
	};

	dis_lookup = {
		{0x00, "NOP"			}, {0x01, "LD BC, d16"	}, {0x02, "LD (BC), A"	}, {0x03, "INC BC"		}, {0x04, "INC B"		}, {0x05, "DEC B"		}, {0x06, "LD B, d8"	}, {0x07, "RLCA"		},
		{0x10, "STOP"			}, {0x11, "LD DE, d16"	}, {0x12, "LD (DE), A"	}, {0x13, "INC DE"		}, {0x14, "INC D"		}, {0x15, "DEC D"		}, {0x16, "LD D, d8"	}, {0x17, "RLA"			},
		{0x20, "JR NZ, r8"		}, {0x21, "LD HL, d16"	}, {0x22, "LD (HL+), A"	}, {0x23, "INC HL"		}, {0x24, "INC H"		}, {0x25, "DEC H"		}, {0x26, "LD H, d8"	}, {0x27, "DAA"			},
		{0x30, "JR NC, r8"		}, {0x31, "LD SP, d16"	}, {0x32, "LD (HL-), A"	}, {0x33, "INC SP"		}, {0x34, "INC (HL)"	}, {0x35, "DEC (HL)"	}, {0x36, "LD (HL), d8"	}, {0x37, "SCF"			},
		{0x40, "LD B, B"		}, {0x41, "LD B, C"		}, {0x42, "LD B, D"		}, {0x43, "LD B, E"		}, {0x44, "LD B, H"		}, {0x45, "LD B, L"		}, {0x46, "LD B, (HL)"	}, {0x47, "LD B, A"		},
		{0x50, "LD D, B"		}, {0x51, "LD D, C"		}, {0x52, "LD D, D"		}, {0x53, "LD D, E"		}, {0x54, "LD D, H"		}, {0x55, "LD D, L"		}, {0x56, "LD D, (HL)"	}, {0x57, "LD D, A"		},
		{0x60, "LD H, B"		}, {0x61, "LD H, C"		}, {0x62, "LD H, D"		}, {0x63, "LD H, E"		}, {0x64, "LD H, H"		}, {0x65, "LD H, L"		}, {0x66, "LD H, (HL)"	}, {0x67, "LD H, A"		},
		{0x70, "LD (HL), B"		}, {0x71, "LD (HL), C"	}, {0x72, "LD (HL), D"	}, {0x73, "LD (HL), E"	}, {0x74, "LD (HL), H"	}, {0x75, "LD (HL), L"	}, {0x76, "HALT"		}, {0x77, "LD (HL), A"	},

		{0x08, "LD (a16), SP"	}, {0x09, "ADD HL, BC"	}, {0x0A, "LD A, (BC)"	}, {0x0B, "DEC BC"		}, {0x0C, "INC C"		}, {0x0D, "DEC C"		}, {0x0E, "LD C, d8"	}, {0x0F, "RRCA"		},
		{0x18, "JR r8"			}, {0x19, "ADD HL, DE"	}, {0x1A, "LD A, (DE)"	}, {0x1B, "DEC DE"		}, {0x1C, "INC E"		}, {0x1D, "DEC E"		}, {0x1E, "LD E, d8"	}, {0x1F, "RRA"			},
		{0x28, "JR Z, r8"		}, {0x29, "ADD HL, HL"	}, {0x2A, "LD A, (HL+)"	}, {0x2B, "DEC HL"		}, {0x2C, "INC L"		}, {0x2D, "DEC L"		}, {0x2E, "LD L, d8"	}, {0x2F, "CPL"			},
		{0x38, "JR C, r8"		}, {0x39, "ADD HL, SP"	}, {0x3A, "LD A, (HL-)"	}, {0x3B, "DEC SP"		}, {0x3C, "INC A"		}, {0x3D, "DEC A"		}, {0x3E, "LD A, d8"	}, {0x3F, "CCF"			},
		{0x48, "LD C, B"		}, {0x49, "LD C, C"		}, {0x4A, "LD C, D"		}, {0x4B, "LD C, E"		}, {0x4C, "LD C, H"		}, {0x4D, "LD C, L"		}, {0x4E, "LD C, (HL)"	}, {0x4F, "LD C, A"		},
		{0x58, "LD E, B"		}, {0x59, "LD E, C"		}, {0x5A, "LD E, D"		}, {0x5B, "LD E, E"		}, {0x5C, "LD E, H"		}, {0x5D, "LD E, L"		}, {0x5E, "LD E, (HL)"	}, {0x5F, "LD E, A"		},
		{0x68, "LD L, B"		}, {0x69, "LD L, C"		}, {0x6A, "LD L, D"		}, {0x6B, "LD L, E"		}, {0x6C, "LD L, H"		}, {0x6D, "LD L, L"		}, {0x6E, "LD L, (HL)"	}, {0x6F, "LD L, A"		},
		{0x78, "LD A, B"		}, {0x79, "LD A, C"		}, {0x7A, "LD A, D"		}, {0x7B, "LD A, E"		}, {0x7C, "LD A, H"		}, {0x7D, "LD A, L"		}, {0x7E, "LD A, (HL)"	}, {0x7F, "LD A, A"		},

		{0x80, "ADD A, B"		}, {0x81, "ADD A, C"	}, {0x82, "ADD A, D"	}, {0x83, "ADD A, E"	}, {0x84, "ADD A, H"	}, {0x85, "ADD A, L"	}, {0x86, "ADD A, (HL)"	}, {0x87, "ADD A, A"	},
		{0x90, "SUB B"			}, {0x91, "SUB C"		}, {0x92, "SUB D"		}, {0x93, "SUB E"		}, {0x94, "SUB H"		}, {0x95, "SUB L"		}, {0x96, "SUB (HL)"	}, {0x97, "SUB A"		},
		{0xA0, "AND B"			}, {0xA1, "AND C"		}, {0xA2, "AND D"		}, {0xA3, "AND E"		}, {0xA4, "AND H"		}, {0xA5, "AND L"		}, {0xA6, "AND (HL)"	}, {0xA7, "AND A"		},
		{0xB0, "OR B"			}, {0xB1, "OR C"		}, {0xB2, "OR D"		}, {0xB3, "OR E"		}, {0xB4, "OR H"		}, {0xB5, "OR L"		}, {0xB6, "OR (HL)"		}, {0xB7, "OR A"		},
		{0xC0, "RET NZ"			}, {0xC1, "POP BC"		}, {0xC2, "JP NZ, a16"	}, {0xC3, "JP a16"		}, {0xC4, "CALL NZ, a16"}, {0xC5, "PUSH BC"		}, {0xC6, "ADD A, d8"	}, {0xC7, "RST 00H"		},
		{0xD0, "RET NC"			}, {0xD1, "POP DE"		}, {0xD2, "JP NC, a16"	}, {0xD3, "NULL"		}, {0xD4, "CALL NC, a16"}, {0xD5, "PUSH DE"		}, {0xD6, "SUB d8"		}, {0xD7, "RST 10H"		},
		{0xE0, "LDH (a8), A"	}, {0xE1, "POP HL"		}, {0xE2, "LD (C), A"	}, {0xE3, "NULL"		}, {0xE4, "NULL"		}, {0xE5, "PUSH HL"		}, {0xE6, "AND d8"		}, {0xE7, "RST 20H"		},
		{0xF0, "LDH A, (a8)"	}, {0xF1, "POP AF"		}, {0xF2, "LD A, (C)"	}, {0xF3, "DI"			}, {0xF4, "NULL"		}, {0xF5, "PUSH AF"		}, {0xF6, "OR d8"		}, {0xF7, "RST 30H"		},

		{0x88, "ADC A, B"		}, {0x89, "ADC A, C"	}, {0x8A, "ADC A, D"	}, {0x8B, "ADC A, E"	}, {0x8C, "ADC A, H"	}, {0x8D, "ADC A, L"	}, {0x8E, "ADC A, (HL)"	}, {0x8F, "ADC A, A"	},
		{0x98, "SBC B"			}, {0x99, "SBC C"		}, {0x9A, "SBC D"		}, {0x9B, "SBC E"		}, {0x9C, "SBC H"		}, {0x9D, "SBC L"		}, {0x9E, "SBC (HL)"	}, {0x9F, "SBC A"		},
		{0xA8, "XOR B"			}, {0xA9, "XOR C"		}, {0xAA, "XOR D"		}, {0xAB, "XOR E"		}, {0xAC, "XOR H"		}, {0xAD, "XOR L"		}, {0xAE, "XOR (HL)"	}, {0xAF, "XOR A"		},
		{0xB8, "CP B"			}, {0xB9, "CP C"		}, {0xBA, "CP D"		}, {0xBB, "CP E"		}, {0xBC, "CP H"		}, {0xBD, "CP L"		}, {0xBE, "CP (HL)"		}, {0xBF, "CP A"		},
		{0xC8, "RET Z"			}, {0xC9, "RET"			}, {0xCA, "JP Z, a16"	}, {0xCB, "CB"			}, {0xCC, "CALL Z, a16"	}, {0xCD, "CALL a16"	}, {0xCE, "ADC A, d8"	}, {0xCF, "RST 08H"		},
		{0xD8, "RET C"			}, {0xD9, "RETI"		}, {0xDA, "JP C, a16"	}, {0xDB, "NULL"		}, {0xDC, "CALL C, a16"	}, {0xDD, "NULL"		}, {0xDE, "SBC d8"		}, {0xDF, "RST 18H"		},
		{0xE8, "ADD SP, r8"		}, {0xE9, "JP HL"		}, {0xEA, "LD (a16), A"	}, {0xEB, "NULL"		}, {0xEC, "NULL"		}, {0xED, "NULL"		}, {0xEE, "XOR d8"		}, {0xEF, "RST 28H"		},
		{0xF8, "LD HL, SP + r8"	}, {0xF9, "LD SP, HL, "	}, {0xFA, "LD A, (a16)"	}, {0xFB, "EI"			}, {0xFC, "NULL"		}, {0xFD, "NULL"		}, {0xFE, "CP d8"		}, {0xFF, "RST 38H"		},
	};
}

CPU::~CPU() {
}

void CPU::write(uint16_t addr, uint8_t data) {
	bus->write(addr, data);
}

uint8_t CPU::read(uint16_t addr) {
	return bus->read(addr);
}

void CPU::clock() {
	// If cycles remaining for an instruction is 0, read next byte
	if (cycles == 0) {
		// First check interrupt handler and return if there has been one
		/*cycles = interrupt_handler();
		if (cycles > 0) {
			cycles--;
			return;
		}*/

		opcode = read(pc);
		
		if (print_toggle) {
			printf("0x%04x: 0x%02x ", pc, opcode);
			printf("%-15s ", dis_lookup[opcode].c_str());
			printf("a: 0x%02x f: 0x%02x b: 0x%02x c: 0x%02x d: 0x%02x e: 0x%02x h: 0x%02x l: 0x%02x pc: 0x%04x sp: 0x%04x\n", a, f, b, c, d, e, h, l, pc, sp);
			//printf("Z: %i N: %i H: %i C: %i \n", getFlag(Z), getFlag(N), getFlag(H), getFlag(C));
			//printf("LY: 0x%02x ", bus->read(0xFF44));
			//printf("gc: %i\n", global_cycles);
		}

		if (log_toggle) {
			fprintf(file, "A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X SP: %04X PC: 00:%04X\n", a, f, b, c, d, e, h, l, sp, pc);
		}

		pc++;

		if (lookup.count(opcode)) {

			// If last instruction was EI, set IME after this instruction is finished
			if (pending_ime) {
				set_ime = true;
				pending_ime = false;
			}
			
			// Set cycles to number of cycles
			cycles = lookup[opcode].cycles;

			// Perform operation
			uint8_t extra_cycles = (this->*lookup[opcode].operate)();

			cycles += extra_cycles;

			// multiply cycles by 4 for correct clock cpu timing
			//cycles *= 4;

			// Set ime flag after instruction following EI
			if (set_ime) {
				IME = true;
			}
		}
		else {
			printf("Unexpected opcode 0x%02x at 0x%04x\n", opcode, pc);
			//pc++;
		}
	}
	cycles--;

	if (print_toggle) {
		printf("                                                                                                                    LY: 0x%02x ", bus->read(0xFF44));
		printf("gc: %i\n", global_cycles);
	}

	global_cycles++;

	// print test rom output
	print_test();
}

void CPU::print_test() {
	// print the test results from blargg's test rom
	if (bus->read(0xFF02) == 0x81) {
		char c = bus->read(0xFF01);
		printf("%c", c);
		bus->write(0xFF02, 0x00);
	}
}

bool CPU::complete() {
	return (cycles == 0);
}

uint8_t CPU::getFlag(FLAGS flag) {
	// return true if f has a 1 in the position of flag
	return ((f & flag) > 0) ? 1 : 0;
}
void CPU::setFlag(FLAGS flag, bool value) {
	// set the position of the flag in f to 1 or 0
	if (value) {
		f |= flag;
	}
	else {
		f &= ~flag;
	}
}

bool CPU::halfCarryPredicate(uint16_t val1, uint16_t val2, uint16_t val3) {
	// half carry
	// if the bottom 4 bits of each added together sets an upper 4 bit
	// added an optional third value to accomodate adc and sbc
	return ((val1 & 0xF) + (val2 & 0xF) + (val3 & 0xF)) & 0x10;
}

bool CPU::carryPredicate(uint16_t val1, uint16_t val2, uint16_t val3) {
	// carry
	// if the bottom 8 bits of each added together sets an upper 8 bit
	return ((val1 & 0xFF) + (val2 & 0xFF) + (val3 & 0xFF)) & 0x100;
}

bool CPU::halfBorrowPredicate(uint16_t val1, uint16_t val2) {
	// half borrow
	// if the borrowing from the 4th bit (if register > a)
	return ((val1 & 0xF) < (val2 & 0xF));
}

bool CPU::borrowPredicate(uint16_t val1, uint16_t val2) {
	// borrow
	// if the borrowing from the theoretical 8th bit (if register > a)
	return ((val1 & 0xFF) < (val2 & 0xFF));
}

bool CPU::halfCarryPredicate16(uint16_t val1, uint16_t val2) {
	// half carry for 16 bit addition
	// if overflow from bit 11
	return ((val1 & 0x0FFF) + (val2 & 0x0FFF)) & 0x1000;
}

bool CPU::carryPredicate16(uint16_t val1, uint16_t val2) {
	// carry for 16 bit addition
	// if overflow from bit 15
	return (((uint32_t)val1 & 0xFFFF) + ((uint32_t)val2 & 0xFFFF)) & 0x10000;
}

bool CPU::checkCondition(uint8_t cc) {
	// return true if jump should occur given a condition code and false if not
	return (cc == CONDITION::c_N)
		|| (cc == CONDITION::c_Z && getFlag(Z))
		|| (cc == CONDITION::c_NZ && !getFlag(Z))
		|| (cc == CONDITION::c_C && getFlag(C))
		|| (cc == CONDITION::c_NC && !getFlag(C));
}

uint8_t CPU::LD_r16() {
	// load 16-bit register
	// d16 - immediate little endian 16 bit data
	// eg LD BC, d16

	// todo: should I check for valid input?

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	*(lookup[opcode].op1) = hi;
	*(lookup[opcode].op2) = lo;

	// apparently b, d, and h hold the more significant values
	// todo: double check this, im not sure why endianness would change
	// https://stackoverflow.com/questions/21639597/z80-register-endianness

	return 0;
}

uint8_t CPU::LD_SP() {
	// load stack pointer with (op2, op3) if provided or immediate 16-bit data if not
	// eg LD SP, HL

	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	if (op1 && op2) {
		// load hl into sp
		sp = (*op1 << 8) | *op2;
	}
	else {
		// get immediate
		uint8_t lo = bus->read(pc);
		pc++;
		uint8_t hi = bus->read(pc);
		pc++;

		// combine and store
		sp = (hi << 8) | lo;
	}

	return 0;
}

uint8_t CPU::LD_a16_SP() {
	// load SP into a16 location
	// a16 - little endian 16 bit address
	// 5 machine cycles
	// no flags

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;
	
	// combine
	uint16_t addr = (hi << 8) | lo;

	// store stack pointer low in addr and high in addr+1
	uint8_t sp_lo = sp & 0xFF;
	uint8_t sp_hi = (sp >> 8) & 0xFF;
	
	bus->write(addr, sp_lo);
	bus->write(addr + 1, sp_hi);

	return 0;
}

uint8_t CPU::LD_HL_SP_o8() {
	// load sp with a signed offset into hl
	// 3 machine cycles
	// 0 0 H C
	
	// Get signed offset
	uint16_t offset = bus->read(pc);
	pc++;

	// If highest bit is negative sign, extend to higher 8 bits
	if (offset & 0x80) {
		offset |= 0xFF00;
	}

	// I think just add, doesn't matter if it extends the page at least for this load
	// since its not actually loading a value from memory other than the offset (not like a branch)
	uint16_t value = sp + offset;

	l = value & 0xFF;
	h = (value >> 8) & 0xFF;

	setFlag(Z, 0);
	setFlag(N, 0);
	setFlag(H, halfCarryPredicate(sp, offset));
	setFlag(C, carryPredicate(sp, offset));
	
	return 0;
}

// B 8-bit loads

uint8_t CPU::LD_r8_r8() {
	// load an 8 bit register or data into an 8 bit register (op2 = null means data)
	// eg LD B, B

	uint8_t *op1 = lookup[opcode].op1;
	uint8_t *op2 = lookup[opcode].op2;

	// todo: might need to check that op1 isn't null

	if (op2) {
		// todo: might need to guard against self assignment
		// todo: double check I can do this with pointers
		*op1 = *op2;
	}
	else {
		*op1 = bus->read(pc);
		pc++;
	}

	return 0;
}

uint8_t CPU::LD_r8_p16() {
	// load the value at the address pointed to by a 16-bit register into an 8-bit register
	// eg LD B, (HL)

	// set value of (hl) to register
	uint8_t hi = *(lookup[opcode].op2);
	uint8_t lo = *(lookup[opcode].op3);

	uint16_t addr = (hi << 8) | lo;
	*(lookup[opcode].op1) = bus->read(addr);

	return 0;
}

uint8_t CPU::LD_p16_r8() {
	// load the value of an 8-bit register into the address pointed to by a 16-bit register
	// eg LD (HL), B
	
	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;
	
	uint8_t hi = *op1;
	uint8_t lo = *op2;

	uint16_t addr = (hi << 8) | lo;

	// if op3 isn't null, use it as the data, otherwise read next value from pc
	uint8_t data = 0x00;
	if (op3) {
		data = *op3;
	}
	else {
		data = bus->read(pc);
		pc++;
	}

	bus->write(addr, data);

	return 0;
}

uint8_t CPU::LD_HLI_A() {
	// eg LD (HL+), A

	// todo: can this overflow?

	uint16_t addr = (h << 8) | l;
	bus->write(addr, a);
	addr++;

	// convert back to h and l
	// idea: function to convert to and from hl and address
	l = addr & 0xFF;
	h = (addr >> 8) & 0xFF;

	return 0;
}

uint8_t CPU::LD_HLD_A() {
	// eg LD (HL-), A

	// todo: can this underflow?
	
	uint16_t addr = (h << 8) | l;
	bus->write(addr, a);
	addr--;

	// convert back to h and l
	l = addr & 0xFF;
	h = (addr >> 8) & 0xFF;

	return 0;
}

uint8_t CPU::LD_A_HLI() {
	// eg LD A, (HL+)

	// note: these are guaranteed to be a, h, and l
	// todo: can this overflow?
	uint16_t addr = (h << 8) | l;
	a = bus->read(addr);
	addr++;

	// convert back to h and l
	l = addr & 0xFF;
	h = (addr >> 8) & 0xFF;

	return 0;
}

uint8_t CPU::LD_A_HLD() {
	// eg LD A, (HL+)

	// todo: can this underflow?

	uint16_t addr = (h << 8) | l;
	a = bus->read(addr);
	addr--;

	// convert back to h and l
	l = addr & 0xFF;
	h = (addr >> 8) & 0xFF;

	return 0;
}

uint8_t CPU::LDH_A_p8() {
	// load A with 0xFF00 page byte from register or address
	// eg LD A, [C]
	// op1 = &c for [C] or null for a8

	uint8_t* op1 = lookup[opcode].op1;

	uint8_t offset;

	if (op1) {
		// load from register
		offset = *op1;
	}
	else {
		// load from immediate address
		offset = bus->read(pc);
		pc++;
	}

	uint16_t addr = 0xFF00 + offset;
	
	a = bus->read(addr);

	return 0;
}

uint8_t CPU::LDH_p8_A() {
	// load 0xFF00 page byte from register or address with A
	// eg LD [C], A
	// op1 = &c for [C] or null for a8

	uint8_t* op1 = lookup[opcode].op1;

	uint8_t offset;

	if (op1) {
		// load from register
		offset = *op1;
	}
	else {
		// load from immediate address
		offset = bus->read(pc);
		pc++;
	}

	uint16_t addr = 0xFF00 + offset;

	// write A to address
	bus->write(addr, a);

	return 0;
}

uint8_t CPU::LD_a16_A() {
	// load A into a16 location
	// a16 - little endian 16 bit address
	// 3 machine cycles
	// no flags

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	// combine
	uint16_t addr = (hi << 8) | lo;

	bus->write(addr, a);

	return 0;
}

uint8_t CPU::LD_A_a16() {
	// load data at a16 location into A
	// a16 - little endian 16 bit address
	// 3 machine cycles
	// no flags

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	// combine
	uint16_t addr = (hi << 8) | lo;

	a = bus->read(addr);

	return 0;
}

uint8_t CPU::PUSH_r16() {
	// push register onto the stack
	// eg PUSH BC

	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	sp--;
	bus->write(sp, *op1);
	sp--;
	bus->write(sp, *op2);

	return 0;
}

uint8_t CPU::PUSH_AF() {
	// PUSH AF has slightly different behaviour because of the flags register

	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	sp--;
	bus->write(sp, *op1);
	sp--;
	bus->write(sp, *op2);

	return 0;
}

uint8_t CPU::POP_r16() {
	// pop register off stack
	// eg POP BC

	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	*op2 = bus->read(sp);
	sp++;
	*op1 = bus->read(sp);
	sp++;

	return 0;
}

uint8_t CPU::POP_AF() {
	// f only cares about upper 4 bits

	f = bus->read(sp) & 0xF0;
	sp++;
	a = bus->read(sp);
	sp++;

	return 0;
}

uint8_t CPU::ADD() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;
	
	// upcast to 16-bit for easier addition
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;

	if (op3) {
		// add from address
		uint16_t addr = (*op2 << 8) | *op3;
		val2 = bus->read(addr);			
	}
	else if (op2) {
		// add from register
		val2 = *op2;
	}
	else {
		// add from immediate
		val2 = bus->read(pc);
		pc++;
	}

	// add and load into a (8-bit)
	*op1 = (val1 + val2) & 0xFF;
	
	// set flags
	setFlag(Z, (*op1 == 0));
	setFlag(N, 0);
	setFlag(H, halfCarryPredicate(val1, val2));
	setFlag(C, carryPredicate(val1, val2));

	return 0;
}

uint8_t CPU::ADC() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;

	// upcast to 16-bit for easier addition
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;

	if (op3) {
		// add from address
		uint16_t addr = (*op2 << 8) | *op3;
		val2 = bus->read(addr);
	}
	else if (op2) {
		// add from register
		val2 = *op2;
	}
	else {
		// add from immediate
		val2 = bus->read(pc);
		pc++;
	}

	// add the carry bit
	uint16_t carry = getFlag(FLAGS::C);

	// add with carry and load into a (8-bit)
	*op1 = (val1 + val2 + carry) & 0xFF;

	// set flags
	setFlag(Z, (*op1 == 0));
	setFlag(N, 0);
	setFlag(H, halfCarryPredicate(val1, val2, carry));
	setFlag(C, carryPredicate(val1, val2, carry));

	return 0;
}

uint8_t CPU::SUB() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;

	// upcast to 16-bit for easier operation
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;

	if (op3) {
		// address
		uint16_t addr = (*op2 << 8) | *op3;
		val2 = bus->read(addr);
	}
	else if (op2) {
		// register
		val2 = *op2;
	}
	else {
		// immediate
		val2 = bus->read(pc);
		pc++;
	}

	// get the twos complement (of the bottom 8 bits)
	uint16_t val2_twos = ~val2 + 0x0001;

	// add twos complement and load into a (8-bit)
	*op1 = (val1 + val2_twos) & 0xFF;

	// set flags
	setFlag(Z, (*op1 == 0));
	setFlag(N, 1);
	setFlag(H, halfBorrowPredicate(val1, val2));
	setFlag(C, borrowPredicate(val1, val2));

	return 0;
}

uint8_t CPU::SBC() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;

	// upcast to 16-bit for easier operation
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;

	if (op3) {
		// address
		uint16_t addr = (*op2 << 8) | *op3;
		val2 = bus->read(addr);
	}
	else if (op2) {
		// register
		val2 = *op2;
	}
	else {
		// immediate
		val2 = bus->read(pc);
		pc++;
	}

	// add the carry bit
	// todo: watch for possible bug with carry bit and sign extension
	uint16_t carry = getFlag(FLAGS::C);

	if (carry) {
		printf("");
	}

	// todo: tech debt, refactor this
	// check for a half carry for the 00-(0F+1) case
	bool halfCarryFlag = halfCarryPredicate(val2, carry);
	bool carryFlag = carryPredicate(val2, carry);

	// add carry before twos complement I think - ultimately subtract
	val2 += carry;

	// get the twos complement (of the bottom 8 bits)
	uint16_t val2_twos = ~val2 + 0x0001;

	// add twos complement and carry and load into a (8-bit)
	*op1 = (val1 + val2_twos) & 0xFF;

	// set flags
	setFlag(Z, (*op1 == 0));
	setFlag(N, 1);
	setFlag(H, halfBorrowPredicate(val1, val2) || halfCarryFlag);
	setFlag(C, borrowPredicate(val1, val2) || carryFlag);

	if (carry) {
		printf("");
	}

	return 0;
}

uint8_t CPU::AND() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;

	// upcast to 16-bit for easier addition
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;

	if (op3) {
		// add from address
		uint16_t addr = (*op2 << 8) | *op3;
		val2 = bus->read(addr);
	}
	else if (op2) {
		// add from register
		val2 = *op2;
	}
	else {
		// add from immediate
		val2 = bus->read(pc);
		pc++;
	}

	// and and load into a (8-bit)
	*op1 = (val1 & val2) & 0xFF;

	// set flags
	setFlag(Z, (*op1 == 0));
	setFlag(N, 0);
	setFlag(H, 1);
	setFlag(C, 0);

	return 0;
}

uint8_t CPU::XOR() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;

	// upcast to 16-bit for easier addition
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;

	if (op3) {
		// add from address
		uint16_t addr = (*op2 << 8) | *op3;
		val2 = bus->read(addr);
	}
	else if (op2) {
		// add from register
		val2 = *op2;
	}
	else {
		// add from immediate
		val2 = bus->read(pc);
		pc++;
	}

	// xor and load into a (8-bit)
	*op1 = (val1 ^ val2) & 0xFF;

	// set flags
	setFlag(Z, (*op1 == 0));
	setFlag(N, 0);
	setFlag(H, 0);
	setFlag(C, 0);

	return 0;
}

uint8_t CPU::OR() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;

	// upcast to 16-bit for easier addition
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;

	if (op3) {
		// add from address
		uint16_t addr = (*op2 << 8) | *op3;
		val2 = bus->read(addr);
	}
	else if (op2) {
		// add from register
		val2 = *op2;
	}
	else {
		// add from immediate
		val2 = bus->read(pc);
		pc++;
	}

	// or and load into a (8-bit)
	*op1 = (val1 | val2) & 0xFF;

	// set flags
	setFlag(Z, (*op1 == 0));
	setFlag(N, 0);
	setFlag(H, 0);
	setFlag(C, 0);

	return 0;
}

uint8_t CPU::CP() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;

	// upcast to 16-bit for easier operation
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;

	if (op3) {
		// address
		uint16_t addr = (*op2 << 8) | *op3;
		val2 = bus->read(addr);
	}
	else if (op2) {
		// register
		val2 = *op2;
	}
	else {
		// immediate
		val2 = bus->read(pc);
		pc++;
	}

	// If highest bit is negative sign, extend to higher 8 bits
	/*if (val2 & 0x80) {
		val2 |= 0xFF00;
	}*/

	//uint8_t temp = (val1 + val2) & 0xFF;

	// get the twos complement (of the bottom 8 bits)
	uint16_t val2_twos = ~val2 + 0x0001;

	// add twos complement and load into temp to set flags
	uint8_t temp = (val1 + val2_twos) & 0xFF;

	// set flags
	// todo: fix/double-check flags for sub a, a
	setFlag(Z, (temp == 0));
	setFlag(N, 1);
	setFlag(H, halfBorrowPredicate(val1, val2));
	setFlag(C, borrowPredicate(val1, val2));
	//setFlag(H, halfCarryPredicate(val1, val2_twos));
	//setFlag(C, carryPredicate(val1, val2_twos));

	return 0;
}

uint8_t CPU::INC() {
	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	// store value to be incremented in temp for flags
	uint16_t temp = 0x0000;
	uint8_t result = 0x00;

	if (op2) {
		// get byte from address
		uint16_t addr = (*op1 << 8) | *op2;
		temp = bus->read(addr);
		result = (temp + 0x0001) & 0xFF;
		bus->write(addr, result);
	}
	else {
		// get byte from register
		temp = *op1;
		result = (temp + 0x0001) & 0xFF;
		*op1 = result;
	}

	// set flags
	setFlag(Z, (result == 0));
	setFlag(N, 0);
	setFlag(H, halfCarryPredicate(temp, 0x0001));

	return 0;
}

uint8_t CPU::DEC() {
	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	// store value to be incremented in temp for flags
	uint16_t temp = 0x0000;
	uint8_t result = 0x00;

	if (op2) {
		// get byte from address
		uint16_t addr = (*op1 << 8) | *op2;
		temp = bus->read(addr);
		result = (temp - 0x0001) & 0xFF;
		bus->write(addr, result);
	}
	else {
		// get byte from register
		temp = *op1;
		result = (temp - 0x0001) & 0xFF;
		*op1 = result;
	}

	// set flags
	setFlag(Z, (result == 0));
	setFlag(N, 1);
	setFlag(H, halfBorrowPredicate(temp, 0x0001));

	return 0;
}

uint8_t CPU::DAA() {
	if (getFlag(N) == 0) {
		// if addition, greater than 9 or carry (since thats also greater than 9)
		if (a > 0x99 || getFlag(C)) {
			a += 0x60;
			setFlag(C, 1);
		}
		if ((a & 0x0F) > 0x09 || getFlag(H)) {
			a += 0x06;
		}
	}

	// if subtraction, assuming bcd operands, result can't be greater than 9 unless there was a borrow
	else {
		if (getFlag(C)) {
			a -= 0x60;
		}
		if (getFlag(H)) {
			a -= 0x06;
		}
	}

	// set flags
	setFlag(Z, (a == 0));
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::SCF() {
	// set carry flag
	setFlag(N, 0);
	setFlag(H, 0);
	setFlag(C, 1);

	return 0;
}

uint8_t CPU::CPL() {
	// complement a
	a = ~a;
	setFlag(N, 1);
	setFlag(H, 1);

	return 0;
}

uint8_t CPU::CCF() {
	// complement carry flag
	setFlag(N, 0);
	setFlag(H, 0);
	setFlag(C, ~getFlag(C));

	return 0;
}

uint8_t CPU::JP() {
	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;
	
	// op1 - condition code
	// op2 and op3 - hl if present, imm if none

	uint16_t addr = 0x0000;

	if (op2 && op3) {
		// get from register
		addr = (*op2 << 8) | *op3;
	}
	else {
		// get from imm
		uint8_t lo = bus->read(pc);
		pc++;
		uint8_t hi = bus->read(pc);
		pc++;

		addr = (hi << 8) | lo;
	}

	// jump if condition is met
	if (checkCondition(*op1)) {
		pc = addr;
		
		// return 1 for extra cycle if branch is taken
		return 1;
	}

	return 0;
}

uint8_t CPU::JR() {
	uint8_t* op1 = lookup[opcode].op1;
	
	// Get signed offset in 16-bits for easier math
	uint16_t offset = bus->read(pc);
	pc++;

	// If highest bit is negative sign, extend to higher 8 bits
	if (offset & 0x80) {
		offset |= 0xFF00;
	}

	if (checkCondition(*op1)) {
		pc += offset;

		// return 1 for extra cycle if branch is taken
		return 1;
		//return 0;
	}

	return 0;
}

uint8_t CPU::CALL() {
	// op1 is condition
	uint8_t* op1 = lookup[opcode].op1;

	if (checkCondition(*op1)) {
		// push next instruction address on stack
		uint16_t next = pc + 2;
		uint8_t next_lo = next & 0xFF;
		uint8_t next_hi = (next >> 8) & 0xFF;
		
		// todo: double check order in terms of endianness
		sp--;
		bus->write(sp, next_hi);
		sp--;
		bus->write(sp, next_lo);

		// execute jump
		// get from imm
		uint8_t lo = bus->read(pc);
		pc++;
		uint8_t hi = bus->read(pc);
		pc++;
		
		pc = (hi << 8) | lo;

		return 3;
	}

	// Skip to the next instruction if call not taken
	pc += 2;

	return 0;
}

uint8_t CPU::RET() {
	// op1 is condition
	uint8_t* op1 = lookup[opcode].op1;

	if (checkCondition(*op1)) {
		// pop address off the stack and set pc
		// should fill upper bits first for this one I think

		uint8_t lo = bus->read(sp);
		sp++;
		uint8_t hi = bus->read(sp);
		sp++;

		pc = (hi << 8) | lo;

		// from 2 cycles, add 2 extra cycles if no condition and 3 if condition
		if (*op1 == CONDITION::c_N) {
			return 2;
		}
		else {
			return 3;
		}
	}

	return 0;
}

uint8_t CPU::RETI() {
	// pop address off the stack and set pc
	uint8_t lo = bus->read(sp);
	sp++;
	uint8_t hi = bus->read(sp);
	sp++;

	pc = (hi << 8) | lo;

	// set IME
	IME = 1;

	return 0;
}

uint8_t CPU::RST() {
	// op1 is vec value
	uint8_t* op1 = lookup[opcode].op1;

	// push next instruction address on stack
	uint16_t next = pc;
	uint8_t next_lo = next & 0xFF;
	uint8_t next_hi = (next >> 8) & 0xFF;

	// todo: double check order in terms of endianness
	sp--;
	bus->write(sp, next_hi);
	sp--;
	bus->write(sp, next_lo);

	// execute jump
	// jump to vec value in op1
	pc = *op1;

	return 0;
}

uint8_t CPU::ADD_r16() {
	// eg ADD HL, BC
	// accumulator always HL

	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	uint16_t val1 = (h << 8) | l;
	uint16_t val2 = (*op1 << 8) | *op2;

	// add and load into hl (8-bit)
	uint16_t sum = (val1 + val2);
	h = (sum >> 8) & 0xFF;
	l = sum & 0xFF;

	// set flags
	setFlag(N, 0);
	setFlag(H, halfCarryPredicate16(val1, val2));
	setFlag(C, carryPredicate16(val1, val2));

	return 0;
}

uint8_t CPU::ADD_SP() {
	// eg ADD HL, SP
	// accumulator always HL

	uint16_t val1 = (h << 8) | l;
	uint16_t val2 = sp;

	// add and load into hl (8-bit)
	uint16_t sum = (val1 + val2);
	h = (sum >> 8) & 0xFF;
	l = sum & 0xFF;

	// set flags
	setFlag(N, 0);
	setFlag(H, halfCarryPredicate16(val1, val2));
	setFlag(C, carryPredicate16(val1, val2));

	return 0;
}

uint8_t CPU::ADD_SP_o8() {
	// eg ADD SP, o8

	// Get signed offset
	uint16_t offset = bus->read(pc);
	pc++;

	// If highest bit is negative sign, extend to higher 8 bits
	if (offset & 0x80) {
		offset |= 0xFF00;
	}

	uint16_t val1 = sp;
	uint16_t val2 = offset;

	// add and load into sp
	sp = val1 + val2;

	// set flags
	setFlag(Z, 0);
	setFlag(N, 0);
	setFlag(H, halfCarryPredicate(val1, val2));
	setFlag(C, carryPredicate(val1, val2));

	return 0;
}

uint8_t CPU::INC_r16() {
	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	uint16_t value = (*op1 << 8) | *op2;
	value++;

	*op1 = (value >> 8) & 0xFF;
	*op2 = value & 0xFF;

	return 0;
}

uint8_t CPU::INC_SP() {
	sp++;
	return 0;
}

uint8_t CPU::DEC_r16() {
	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	uint16_t value = (*op1 << 8) | *op2;
	value--;

	*op1 = (value >> 8) & 0xFF;
	*op2 = value & 0xFF;

	return 0;
}

uint8_t CPU::DEC_SP() {
	sp--;
	return 0;
}

uint8_t CPU::NOP() {
	return 0;
}

uint8_t CPU::EI() {
	// enable IME flag
	// needs to happen after the next instruction
	pending_ime = true;
	return 0;
}

uint8_t CPU::DI() {
	// disable IME flag
	IME = 0;
	return 0;
}

uint8_t CPU::STOP() {
	// ignore next byte
	pc++;

	// todo: implement this

	return 0;
}

uint8_t CPU::HALT() {
	// halt

	if (IME) {
		if (IE & IF) {
			// wake up
			// call interrupt handler
		}
	}
	else {
		if (IE & IF) {
			// halt bug
			
			// normal case - read byte after halt twice
			
			// ei before halt - interrupt serviced, handler called, then interrupt executes another halt
			// and waits for another interrupt
		}
		else {
			// wait until interrupt becomes pending
		}
	}

	return 0;
}

uint8_t CPU::CB() {
	// Read next opcode
	cb_opcode = bus->read(pc);
	pc++;

	// Set cycles to number of cycles in next opcode
	uint8_t cb_cycles = cb_lookup[cb_opcode].cycles;
	
	// Perform operation
	uint8_t extra_cycle = (this->*cb_lookup[cb_opcode].operate)();

	cb_cycles += extra_cycle;

	return cb_cycles;
}

uint8_t CPU::BIT() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	uint8_t* op3 = cb_lookup[cb_opcode].op3;

	// op1 is to shift, op2 is register, op3 is for (HL)

	uint8_t value = 0x00;

	if (op3) {
		// (HL)
		uint16_t addr = (*op2 << 8) | *op3;
		value = bus->read(addr);
	}
	else {
		// register
		value = *op2;
	}
	
	// test bit in value
	bool bit_set = value & (1 << *op1);
	
	setFlag(Z, !bit_set);
	setFlag(N, 0);
	setFlag(H, 1);

	return 0;
}

uint8_t CPU::RES() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	uint8_t* op3 = cb_lookup[cb_opcode].op3;

	// op1 is to shift, op2 is register, op3 is for (HL)

	if (op3) {
		// (HL)
		uint16_t addr = (*op2 << 8) | *op3;
		uint8_t value = bus->read(addr);
		bus->write(addr, value & ~(1 << *op1));
	}
	else {
		// register
		*op2 = *op2 & ~(1 << *op1);
	}

	return 0;
}

uint8_t CPU::SET() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	uint8_t* op3 = cb_lookup[cb_opcode].op3;

	// op1 is to shift, op2 is register, op3 is for (HL)

	if (op3) {
		// (HL)
		uint16_t addr = (*op2 << 8) | *op3;
		uint8_t value = bus->read(addr);
		bus->write(addr, value & (1 << *op1));
	}
	else {
		// register
		*op2 = *op2 & (1 << *op1);
	}

	return 0;
}

uint8_t CPU::RLCA() {
	// cast to 16 bits
	// set the bit 7 to carry
	// shift everything left 1 or right 7 (to wrap around)
	// mask the bottom 8 bits

	uint16_t temp = a;
	setFlag(C, temp & 0x0080);
	temp = (temp << 1) | (temp >> 7);

	a = temp & 0xFF;

	setFlag(Z, 0);
	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::RRCA() {
	// cast to 16 bits
	// set bit 0 to carry
	// shift everything right 1 or left 7 (to wrap around)
	// mask the bottom 8 bits

	uint16_t temp = a;
	setFlag(C, temp & 0x0001);
	temp = (temp >> 1) | (temp << 7);

	a = temp & 0xFF;

	setFlag(Z, 0);
	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::RLA() {
	// cast to 16 bits
	// shift everything left 1
	// set carry to the value of the final bit 8
	// set first bit 0 to the value of carry
	// mask the bottom 8 bits

	uint16_t temp = a;
	temp = temp << 1;
	setFlag(C, temp & 0x0100);
	temp |= getFlag(C);

	a = temp & 0xFF;

	setFlag(Z, 0);
	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::RRA() {
	// cast to 16 bits
	// set final bit 8 to the value of carry (itll be shifted right)
	// set carry to the value of the first bit 0
	// shift everything right 1
	// mask the bottom 8 bits

	uint16_t temp = a;
	temp |= getFlag(C) << 8;
	setFlag(C, temp & 0x0001);
	temp = temp >> 1;

	a = temp & 0xFF;

	setFlag(Z, 0);
	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::RLC() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	// op1 is register, op2 is for (hl)
	
	// todo: double check passing in a derefenced pointer makes a copy of it
	// cast to 16 bits
	// set the bit 7 to carry
	// shift everything left 1 or right 7 (to wrap around)
	// mask the bottom 8 bits
	auto shift = [&](uint16_t value) -> uint8_t {
		setFlag(C, value & 0x0080);
		value = (value << 1) | (value >> 7);
		setFlag(Z, ((value & 0xFF) == 0));
		return (value & 0xFF);
	};
	
	if (op2) {
		uint16_t addr = (*op1 << 8) | *op2;
		uint8_t data = bus->read(addr);
		bus->write(addr, shift(data));
	}
	else {
		*op1 = shift(*op1);
	}

	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::RRC() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	// op1 is register, op2 is for (hl)

	// cast to 16 bits
	// set bit 0 to carry
	// shift everything right 1 or left 7 (to wrap around)
	// mask the bottom 8 bits
	auto shift = [&](uint16_t value) -> uint8_t {
		setFlag(C, value & 0x0001);
		value = (value >> 1) | (value << 7);
		setFlag(Z, ((value & 0xFF) == 0));
		return (value & 0xFF);
	};

	if (op2) {
		uint16_t addr = (*op1 << 8) | *op2;
		uint8_t data = bus->read(addr);
		bus->write(addr, shift(data));
	}
	else {
		*op1 = shift(*op1);
	}

	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::RL() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	// op1 is register, op2 is for (hl)

	// cast to 16 bits
	// shift everything left 1
	// set carry to the value of the final bit 8
	// set first bit 0 to the value of carry
	// mask the bottom 8 bits
	auto shift = [&](uint16_t value) -> uint8_t {
		value = value << 1;
		setFlag(C, value & 0x0100);
		value |= getFlag(C);
		setFlag(Z, ((value & 0xFF) == 0));
		return (value & 0xFF);
	};

	if (op2) {
		uint16_t addr = (*op1 << 8) | *op2;
		uint8_t data = bus->read(addr);
		bus->write(addr, shift(data));
	}
	else {
		*op1 = shift(*op1);
	}

	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::RR() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	// op1 is register, op2 is for (hl)

	// cast to 16 bits
	// set final bit 8 to the value of carry (itll be shifted right)
	// set carry to the value of the first bit 0
	// shift everything right 1
	// mask the bottom 8 bits
	auto shift = [&](uint16_t value) -> uint8_t {
		value |= getFlag(C) << 8;
		setFlag(C, value & 0x0001);
		value = value >> 1;
		setFlag(Z, ((value & 0xFF) == 0));
		return (value & 0xFF);
	};

	if (op2) {
		uint16_t addr = (*op1 << 8) | *op2;
		uint8_t data = bus->read(addr);
		bus->write(addr, shift(data));
	}
	else {
		*op1 = shift(*op1);
	}

	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::SLA() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	// op1 is register, op2 is for (hl)

	// cast to 16 bits
	// shift everything left 1
	// set carry to the value of the final bit 8
	// mask the bottom 8 bits
	auto shift = [&](uint16_t value) -> uint8_t {
		value = value << 1;
		setFlag(C, value & 0x0100);
		setFlag(Z, ((value & 0xFF) == 0));
		return (value & 0xFF);
	};

	if (op2) {
		uint16_t addr = (*op1 << 8) | *op2;
		uint8_t data = bus->read(addr);
		bus->write(addr, shift(data));
	}
	else {
		*op1 = shift(*op1);
	}

	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::SRL() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	// op1 is register, op2 is for (hl)

	// cast to 16 bits
	// set carry to the value of the first bit 0
	// shift everything right 1
	// mask the bottom 8 bits
	auto shift = [&](uint16_t value) -> uint8_t {
		setFlag(C, value & 0x0001);
		value = value >> 1;
		setFlag(Z, ((value & 0xFF) == 0));
		return (value & 0xFF);
	};

	if (op2) {
		uint16_t addr = (*op1 << 8) | *op2;
		uint8_t data = bus->read(addr);
		bus->write(addr, shift(data));
	}
	else {
		*op1 = shift(*op1);
	}

	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::SRA() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	// op1 is register, op2 is for (hl)

	// cast to 16 bits
	// set carry to the value of the first bit 0
	// copy bit 7 into bit 8
	// shift everything right 1
	// mask the bottom 8 bits
	auto shift = [&](uint16_t value) -> uint8_t {
		setFlag(C, value & 0x0001);
		value |= (value & (1 << 7)) << 1;				// mask bit 7, shift it to the left, then add it to value
		value = value >> 1;
		setFlag(Z, ((value & 0xFF) == 0));
		return (value & 0xFF);
	};

	if (op2) {
		uint16_t addr = (*op1 << 8) | *op2;
		uint8_t data = bus->read(addr);
		bus->write(addr, shift(data));
	}
	else {
		*op1 = shift(*op1);
	}

	setFlag(N, 0);
	setFlag(H, 0);

	return 0;
}

uint8_t CPU::SWAP() {
	uint8_t* op1 = cb_lookup[cb_opcode].op1;
	uint8_t* op2 = cb_lookup[cb_opcode].op2;
	// op1 is register, op2 is for (hl)

	// set hi to the bottom 4 bits shifted left by 4 (0xX0)
	// shift value right by 4 (0x0X)
	// combine
	auto swap = [&](uint8_t value) -> uint8_t {
		uint8_t hi = value << 4;
		value >>= 4;
		value |= hi;
		setFlag(Z, (value == 0));
		return value;
	};
	
	if (op2) {
		uint16_t addr = (*op1 << 8) | *op2;
		uint8_t data = bus->read(addr);
		bus->write(addr, swap(data));
	}
	else {
		*op1 = swap(*op1);
	}
	
	setFlag(N, 0);
	setFlag(H, 0);
	setFlag(C, 0);

	return 0;
}

void CPU::simLY() {
	// first check if screen is off and reset everything if so
	if (!(bus->read(0xFF40) & 0x80)) {
		// LCD off
		bus->write(0xFF44, 0x00);
		scanline_clock = 0;

		return;
	}
	
	bool inc = false;
	
	// increment every 114 cycles per scanline
	// add 4 for now to use real cycles, not machine cycles
	scanline_clock++;
	if (scanline_clock > 113) {
		scanline_clock = 0;
		
		inc = true;
	}

	if (inc) {
		// get byte from 0xFF44
		uint16_t addr = 0xFF44;
		uint8_t value = bus->read(addr);

		// reset after 154 cycles
		value++;
		if (value > 153) {
			value = 0x00;
		}

		bus->write(addr, value);
	}
}

uint8_t CPU::interrupt_handler() {
	// read interrupt register
	if (IME == 0) {
		return 0;
	}

	IE = bus->read(0xFFFF);
	IF = bus->read(0xFF0F);

	bool interrupt = false;
	uint8_t int_cycles = 5;

	auto push_pc = [&]() {
		// push current pc to stack
		uint16_t next = pc;
		uint8_t next_lo = next & 0xFF;
		uint8_t next_hi = (next >> 8) & 0xFF;

		sp--;
		bus->write(sp, next_hi);
		sp--;
		bus->write(sp, next_lo);
	};

	// priority in order
	if ((IE & (1 << 0)) && (IF & (1 << 0))) {
		// bit 0, vblank
		// reset IF and IME
		IF &= ~(1 << 0);
		IME = 0;

		push_pc();
		pc = 0x0040;
		return int_cycles;
	}
	else if ((IE & (1 << 1)) && (IF & (1 << 1))) {
		// bit 1, LCD STAT
		IF &= ~(1 << 1);
		IME = 0;

		push_pc();
		pc = 0x0048;
		return int_cycles;
	}
	else if ((IE & (1 << 2)) && (IF & (1 << 2))) {
		// bit 2, Timer
		IF &= ~(1 << 2);
		IME = 0;

		push_pc();
		pc = 0x0050;
		return int_cycles;
	}
	else if ((IE & (1 << 3)) && (IF & (1 << 3))) {
		// bit 3, Serial
		IF &= ~(1 << 3);
		IME = 0;

		push_pc();
		pc = 0x0058;
		return int_cycles;
	}
	else if ((IE & (1 << 4)) && (IF & (1 << 4))) {
		// bit 4, Joypad
		IF &= ~(1 << 4);
		IME = 0;

		push_pc();
		pc = 0x0060;
		return int_cycles;
	}
	
	return 0;
}