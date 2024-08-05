#pragma once

class SM83;
struct Flags;

#include "cpu.h"
#include "memory.h"

enum Mode {reg, byt, wor};

class Debug
{
private:
    /* data */
        SM83& cpu;
    
public:
    Debug(SM83 cpu);
    ~Debug();

    // Runs the debugger
    void run();

    // Runs for testing
    bool runTests(Mode mode, int final_address, int expected_result, Flags flags, bool test_flags);

    // Misc methods for debugging
    
};