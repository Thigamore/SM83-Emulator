#pragma once

class Debug;

#include <stdint.h>
#include <vector>
#include <string>
#include "debug.h"

using byte = uint8_t;
using word = uint16_t;

// Interface for the memory
class Memory {
    public:
        virtual byte getByte(word address) = 0;
        virtual word getWord(word address) = 0;

        virtual void setByte(word address, byte byte) = 0;
        virtual void setWord(word address, word word) = 0;
};

// Interface for simple memory model
// Little Endian
class SimpleMemory: public Memory {
    public:
        friend Debug;

        // Gets a part of memory
        byte getByte(word address);
        word getWord(word address);

        // Sets a part of memory
        void setByte(word address, byte byte);
        void setWord(word address, word word);

        // Load a file into memory
        void loadMemory(std::string filename);

        // Print all of memory (debugging purposes)
        void printMemory();

        SimpleMemory(int size);
        SimpleMemory(std::vector<int> instructions);
        ~SimpleMemory();

    private:
        std::vector<byte> mem;
};