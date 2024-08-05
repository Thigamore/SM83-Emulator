#include "helper.h"
#include <stdint.h>

// Gets from bit begin to end, least to most significant, inclusive
byte getBits(byte byte, int begin, int end)
{
    return byte << 7 - end >> 7 - end + begin;
}

bool getBit(byte num, int bit) {
    return num & (0b1 << bit);
}

byte getDestination(byte instruction)
{
    return (instruction & 0b111000) >> 3;
}

byte getSource(byte instruction)
{
    return instruction & 0b111;
}

int getAltDestination(byte instruction) 
{
    return (instruction & 0b110000) >> 4;
}


word combineBytes(byte left, byte right)
{
    return (((word)left) << 8) + ((word)right);
}
