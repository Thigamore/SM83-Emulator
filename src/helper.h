#pragma once
#include <stdint.h>

using byte = uint8_t;
using word = uint16_t;

byte getBits(byte byte, int begin, int end);
bool getBit(byte num, int bit);

byte getDestination(byte instruction);
byte getSource(byte instruction);

int getAltDestination(byte instruction);

word combineBytes(byte left, byte right);