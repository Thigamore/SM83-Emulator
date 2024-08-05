#include <stdint.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <bitset>
#include <algorithm>
#include "memory.h"

// Gets a byte from memory
byte SimpleMemory::getByte(word address) {
    return this->mem[address];
}

// Get a word from memory
word SimpleMemory::getWord(word address) {
    return (
        ((word) this->mem[address]) + ((word) this->mem[address + 1] << 8)
    );
}

void SimpleMemory::setByte(word address, byte byte) {
    this->mem[address] = byte;
}

void SimpleMemory::setWord(word address, word word) {
    this->mem[address] = word & 0b11111111;
    this->mem[address + 1] = word >> 8;
}

void SimpleMemory::loadMemory(std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if(!file) {
        std::cout << "ERROR OPENING FILE" << std::endl;
        return;
    }
    for(int i = 0; !file.eof() ;i++) {
        file.read((char*) &this->mem[i], sizeof(byte));
    }
    file.close();
}

void SimpleMemory::printMemory()
{
    for(int i = 0; i < this->mem.size(); i++) {
        std::cout << std::bitset<8>(mem[i]) << std::endl;
    }
}

SimpleMemory::SimpleMemory(int size)
{
    this->mem = std::vector<byte>(size);
}

SimpleMemory::SimpleMemory(std::vector<int> instructions)
{
    mem = std::vector<byte>(std::max((int)instructions.size(), 100));
    copy(instructions.begin(), instructions.end(), mem.begin());
}

SimpleMemory::~SimpleMemory() {
    this->mem.clear();
}