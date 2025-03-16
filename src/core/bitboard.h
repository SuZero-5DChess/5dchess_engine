#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <board.h>

// because BOARD_LENGTH is set to 3
// a bitborad should contain 64 bits
using bitboard_t = uint64_t;

struct bb_full_collection
{
    bitboard_t white;
    bitboard_t black;
    bitboard_t royal;
    bitboard_t rook_like;
    bitboard_t bishop_like;
    bitboard_t unicorn_like;
    bitboard_t dragon_like;
    bitboard_t knight;
    bitboard_t pawn;
    bb_full_collection(board b);
};


#endif // BITBOARD_H
