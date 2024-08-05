#include "debug.h"
#include <iostream>
#include <string>

using namespace std;

Debug::Debug(SM83 cpu): cpu(cpu)
{
}

Debug::~Debug()
{
}

// Runs the CPU code instruction by instruction
void Debug::run()
{
    while(true) {
        string command = "";
        while(command != "next") {
            cout << "Enter your command: ";
            getline(cin, command);
            cout << endl;
            if(command == "exit") {
                return;
            } else if(command == "reg") {

            } else {
                cout << "Didn't understand command " << command << ". Please try again." << endl;
            }
        }
    }
}

// Runs a test for a single command
bool Debug::runTests(Mode mode, int final_address, int expected_result, Flags flags, bool test_flags)
{
    // While the instruction doesn't return 0;
    while(cpu.execute() != 0); 
    bool result;
    switch(mode) {
        case reg:
            result = cpu.regs[final_address] == expected_result;
            break;
        case byt:
            result = cpu.mem.getByte(final_address) == expected_result;
            break;
        case wor:
            result = cpu.mem.getWord(final_address) == expected_result;
            break;
    }
    if(test_flags) {
        result = result && cpu.flags.Z == flags.Z && cpu.flags.H == flags.H && cpu.flags.N == flags.N && cpu.flags.C == flags.C;
    }
    return result;
}

