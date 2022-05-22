#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <chrono>

class MMU;

class CPU {
public:
	CPU(MMU* mmu);
	~CPU();

public:
	// CPU core registers
	// Initialized to proper values after boot rom
	uint8_t a = 0x01;	// accumulator
	uint8_t f = 0xB0;	// flags
	uint8_t b = 0x00;
	uint8_t c = 0x13;	// bc
	uint8_t d = 0x00;
	uint8_t e = 0xD8;	// de
	uint8_t h = 0x01;
	uint8_t l = 0x4D;	// hl

	uint16_t sp = 0xFFFE;	// stack pointer
	uint16_t pc = 0x0100;	// program counter

	// Interrupt flag
	uint8_t IME = 0x00;

	// External event functions
	void clock();
	uint8_t timer();
	uint8_t interrupt_handler();
	uint8_t halt_cycle();
	void resetDivider();

	// Signal that instruction is complete
	bool complete();

	// Status register flags
	enum FLAGS {
		Z = (1 << 7),	// zero flag
		N = (1 << 6),	// subtraction flag
		H = (1 << 5),	// half carry flag
		C = (1 << 4)	// carry flag
	};

private:
	// Helper functions to access status register
	uint8_t getFlag(FLAGS flag);
	void setFlag(FLAGS flag, bool value);
	
	// Helper functions for carry logic
	bool halfCarryPredicate(uint16_t val1, uint16_t val2, uint16_t val3 = 0x0000);
	bool carryPredicate(uint16_t val1, uint16_t val2, uint16_t val3 = 0x0000);
	bool halfBorrowPredicate(uint16_t val1, uint16_t val2);
	bool borrowPredicate(uint16_t val1, uint16_t val2);

	bool halfCarryPredicate16(uint16_t val1, uint16_t val2);
	bool carryPredicate16(uint16_t val1, uint16_t val2);
	
	// Helper function to check if a condition is met
	bool checkCondition(uint8_t cc);

	// Helper function for proper EI behaviour
	void handleEI();

	// Helper variables
	uint8_t cycles = 0;
	uint8_t opcode = 0x00;
	uint8_t cb_opcode = 0x00;
	uint16_t addr_abs = 0x0000;
	
	bool set_ime = false;
	bool ei_last_instr = false;
	bool read_next_twice = false;

	bool halt_state = false;
	bool initial_pending_interrupt = false;
	
	uint16_t divider_clock = 0;
	uint16_t timer_clock = 0;
	
	// Enum for condition codes
	// TODO: Should this be an emum class?
	enum CONDITION {
		c_N,		// none
		c_Z,		// zero flag set
		c_NZ,		// zero flag not set
		c_C,		// carry flag set
		c_NC		// carry flag not set
	};

	// Create uint8_t for constant values to pass into opcode functions
	uint8_t rst[8] = { 0x00, 0x10, 0x20, 0x30, 0x08, 0x18, 0x28, 0x38 };
	uint8_t bit[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	uint8_t i_N = CONDITION::c_N;
	uint8_t i_Z = CONDITION::c_Z;
	uint8_t i_NZ = CONDITION::c_NZ;
	uint8_t i_C = CONDITION::c_C;
	uint8_t i_NC = CONDITION::c_NC;

	// Facilitate link to mmu
	MMU* mmu = nullptr;
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	struct INSTRUCTION {
		uint8_t(CPU::* operate)(void) = nullptr;
		uint8_t cycles = 0;
		uint8_t* op1 = nullptr;
		uint8_t* op2 = nullptr;
		uint8_t* op3 = nullptr;
	};

	// Lookup tables for opcodes
	std::map<uint8_t, INSTRUCTION> lookup_map;
	std::map<uint8_t, INSTRUCTION> cb_lookup_map;
	std::map<uint8_t, std::string> name_lookup_map;

	std::vector<INSTRUCTION> lookup;
	std::vector<INSTRUCTION> cb_lookup;
	std::vector<std::string> name_lookup;
	

private:
	// Opcodes

	// r8 - 8-bit register, or data if no operand
	// r16 - 16-bit register
	// d16 - 16 bits of data
	// p16 - address pointed to by a 16-bit register
	// p8 - address pointed to by 8-bit register, OR immediate 8-bit address (for LDH)
	// a16 - immediate 16-bit address
	// o8 - 8-bit signed offset

	// TODO: simplify
	
	uint8_t LD_r16();
	uint8_t LD_SP();

	uint8_t LD_a16_SP();
	uint8_t LD_HL_SP_o8();
	
	uint8_t LD_r8_r8();
	uint8_t LD_r8_p16();
	uint8_t LD_p16_r8();
	
	uint8_t LD_HLI_A();
	uint8_t LD_HLD_A();
	uint8_t LD_A_HLI();
	uint8_t LD_A_HLD();

	uint8_t LDH_A_p8();
	uint8_t LDH_p8_A();
	uint8_t LD_a16_A();
	uint8_t LD_A_a16();

	uint8_t PUSH_r16();
	uint8_t POP_r16();
	uint8_t PUSH_AF();
	uint8_t POP_AF();

	uint8_t ADD();
	uint8_t ADC();
	uint8_t SUB();
	uint8_t SBC();

	uint8_t ADD_r16();
	uint8_t ADD_SP();
	uint8_t ADD_SP_o8();
	
	uint8_t INC_r16();
	uint8_t INC_SP();
	uint8_t DEC_r16();
	uint8_t DEC_SP();

	uint8_t AND();
	uint8_t XOR();
	uint8_t OR();
	uint8_t CP();

	uint8_t INC();
	uint8_t DEC();

	uint8_t DAA();
	uint8_t SCF();
	uint8_t CPL();
	uint8_t CCF();

	uint8_t JP();
	uint8_t JR();
	uint8_t CALL();
	uint8_t RET();
	uint8_t RETI();
	uint8_t RST();

	uint8_t NOP();
	uint8_t STOP();
	uint8_t HALT();
	
	uint8_t EI();
	uint8_t DI();

	uint8_t CB();

	uint8_t BIT();
	uint8_t RES();
	uint8_t SET();
	
	uint8_t RLCA();
	uint8_t RLA();
	uint8_t RRCA();
	uint8_t RRA();

	uint8_t RLC();
	uint8_t RL();
	uint8_t RRC();
	uint8_t RR();

	uint8_t SLA();
	uint8_t SRA();
	uint8_t SWAP();
	uint8_t SRL();

public:
	// Debug and log related things
	bool print_toggle = false;
	bool log_toggle = false;
	std::string log_file = "log.txt";

	FILE* file;
	
	uint64_t global_cycles = 0;

	// Use for simple profiling
	std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	long long time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
	
	long profile_count = 0;
	long profile_total_time = 0;
	long avg = 0;
};
