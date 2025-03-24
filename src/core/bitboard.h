#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <board.h>
#include <string>

// because BOARD_LENGTH is set to 3
// a bitborad should contain 64 bits
using bitboard_t = uint64_t;

/*
The bitboard layout always looks like this:
8.| 56 57 58 59 60 61 62 63
7.| 48 49 50 51 52 53 54 55
6.| 40 41 42 43 44 45 46 47
5.| 32 33 34 35 36 37 38 39
4.| 24 25 26 27 28 29 30 31
3.| 16 17 18 19 20 21 22 23
2.| 08 09 10 11 12 13 14 15
1.| 00 01 02 03 04 05 06 07
  +-------------------------
    a. b. c. d. e. f. g. h.
 */

std::string bitboard_to_string(bitboard_t);

struct bb_full_collection
{
    bitboard_t white;
    bitboard_t black;
    bitboard_t royal;
    bitboard_t king;
    bitboard_t rook;
    bitboard_t bishop;
    bitboard_t unicorn;
    bitboard_t dragon;
    bitboard_t queen;
    bitboard_t knight;
    bitboard_t pawn;
    bb_full_collection(const board& b);
};


#endif // BITBOARD_H
