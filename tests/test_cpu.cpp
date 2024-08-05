#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <iostream>

#include "../src/debug.h"
#include "../src/memory.h"
#include "../src/cpu.h"

using namespace std;


// Tests for the opcodes
TEST_CASE("Test for 8-bit ld") {
    int result = 0b00000010;
    SimpleMemory mem(vector<int>{
        0b00000110,
        result,
        0b01001000,
    });
    SM83 cpu(mem, true);
    Debug debug(cpu);
    mem.printMemory();
    REQUIRE(debug.runTests(reg, 1, result, {}, false));
}

TEST_CASE("Test For LD byte r,n") {
    int result = 0b00000010;
    SimpleMemory mem(vector<int>{
        0b00000110,
        result,
    });
    SM83 cpu(mem, true);
    Debug debug(cpu);
    REQUIRE(debug.runTests(reg,0,result, {}, false));
    mem.printMemory();
}