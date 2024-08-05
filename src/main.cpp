#include <iostream>
#include <string>
#include "memory.h"
#include "cpu.h"

using namespace std;

int main() {
    // 0x09
    SimpleMemory mem(std::vector<int>{
        0x09,
        
    });
    
    SM83 cpu(mem);
    while(true) {
        cpu.execute();
    }
}
