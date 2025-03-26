#ifndef MAGIC_H
#define MAGIC_H

#include "bitboard.h"

bitboard_t rook_attack(int pos, bitboard_t blocker);
bitboard_t bishop_attack(int pos, bitboard_t blocker);
bitboard_t queen_attack(int pos, bitboard_t blocker);

#endif //MAGIC_H
