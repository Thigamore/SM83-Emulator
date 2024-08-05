#include <catch2/catch_test_macros.hpp>
#include "../src/helper.h"

TEST_CASE("Testing GetBits", "[GetBits]") {
    REQUIRE(getBits((uint8_t) 0b00111000,3,5) == 0b111);
    REQUIRE(getBits(0b00000111, 0, 2) == 0b111);
}

TEST_CASE("Testing CombineBytes", "[CombineBytes]") {
    REQUIRE(combineBytes(0b11111111,0b11111111) == 0b1111111111111111);
    REQUIRE(combineBytes(0b10101010, 0b10101010) == 0b1010101010101010);
}