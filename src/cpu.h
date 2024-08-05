#pragma once
#include <stdint.h>
#include <string>

class Debug;
class Memory;

#include "debug.h"
#include "memory.h"

// Types
using byte = uint8_t;
using word = uint16_t;

// Constants for Word Registers
const int BC = 0;
const int DE = 1;
const int HL = 2;
const int SP = 3;

// Constants for Half Registers
const int B = 0;
const int C = 1;
const int D = 2;
const int E = 3;
const int H = 4;
const int L = 5;
const int A = 7;


struct Flags
{
    bool Z : 1, // Zero Flag
        N : 1, // If last instruction was a substraction
        H : 1, // carry of lower 4 bits
        C : 1; // carry of byte/wird
};

class SM83
{
private:
    // Some debugging needs
    friend Debug;
    bool test_mode;

    // Registers
    byte regs[8]; // B, C, D, E, H, L, _, A  (Cuz A can just be called)
    byte  IR;
    byte IE;
    word PC;
    word SP_reg;
    Flags flags;
    bool IME; // Interrupt Master Enable Flag
    bool prevIME; // IME set by IE is delayed by 1 instruction
    
 
    // Memory
    Memory& mem;

public:
    // Constructor and Destructor
    SM83(Memory &mem, bool test_mode = false);
    ~SM83();

    void setHalfRegister(int reg, byte value);
    void setWordRegister(int reg, word value);

    byte getHalfRegister(int reg);
    word getWordRegister(int reg);

    void incrementWordRegister(int reg);
    void decrementWordRegister(int reg);

    void pushStack(word data);
    word popStack();
    
    void DAA();

    // CPU Functions
    int execute();
    int executeCB();
};
