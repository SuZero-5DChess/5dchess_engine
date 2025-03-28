#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <bit>
#include "piece.h"
#include "utils.h"

// because BOARD_LENGTH is set to 3
// a bitborad should contain 64 bits
using bitboard_t = uint64_t;
#define BB_BITS 64

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

std::string bb_to_string(bitboard_t);

constexpr bitboard_t a_file = 0x0101010101010101;
constexpr bitboard_t h_file = 0x8080808080808080;

constexpr bitboard_t shift_north(bitboard_t b)
{
    return b << BOARD_LENGTH;
}
constexpr bitboard_t shift_south(bitboard_t b)
{
    return b >> BOARD_LENGTH;
}
constexpr bitboard_t shift_west(bitboard_t b)
{
    return (b & ~a_file) >> 1;
}
constexpr bitboard_t shift_east(bitboard_t b)
{
    return (b & ~h_file) << 1;
}
constexpr bitboard_t shift_northwest(bitboard_t b)
{
    return (b & ~a_file) << (BOARD_LENGTH - 1);
}
constexpr bitboard_t shift_northeast(bitboard_t b)
{
    return (b & ~a_file) << (BOARD_LENGTH + 1);
}
constexpr bitboard_t shift_southwest(bitboard_t b)
{
    return (b & ~a_file) >> (BOARD_LENGTH + 1);
}
constexpr bitboard_t shift_southeast(bitboard_t b)
{
    return (b & ~a_file) >> (BOARD_LENGTH - 1);
}

constexpr int bb_get_pos(bitboard_t b)
{
    int n = std::countl_zero(b);
    return std::numeric_limits<bitboard_t>::digits - 1 - n;
}
std::vector<int> marked_pos(bitboard_t b);

/*
The bitboards collection associated with a board.
 */
struct boardbits
{
    enum bitboard_indices {
        WHITE, BLACK, ROYAL, //flags
        LKING, LKNIGHT, LPAWN, LRAWN, //jumping pieces
        LROOK, LBISHOP, LUNICORN, LDRAGON, //sliding pieces
        BBS_INDICES_COUNT
    };
    std::array<bitboard_t, BBS_INDICES_COUNT> bbs;
    boardbits() = default;
    boardbits(std::array<piece_t, BOARD_SIZE> b);
    
    // inline getter functions
    constexpr bitboard_t occupied() const
    {
        return bbs[WHITE] | bbs[BLACK];
    }
    constexpr bitboard_t king() const
    {
        return bbs[LKING] & bbs[ROYAL];
    }
    constexpr bitboard_t common_king() const
    {
        return bbs[LKING] & ~bbs[ROYAL];
    }
    constexpr bitboard_t knight() const
    {
        return bbs[LKNIGHT];
    }
    constexpr bitboard_t pawn() const
    {
        return bbs[LPAWN] & ~bbs[LRAWN];
    }
    constexpr bitboard_t brawn() const
    {
        return bbs[LPAWN] & bbs[LRAWN];
    }
    constexpr bitboard_t rook() const
    {
        return bbs[LROOK] & ~bbs[LBISHOP];
    }
    constexpr bitboard_t bishop() const
    {
        return bbs[LBISHOP] & ~bbs[LROOK];
    }
    constexpr bitboard_t unicorn() const
    {
        return bbs[LUNICORN] & ~bbs[LDRAGON];
    }
    constexpr bitboard_t dragon() const
    {
        return bbs[LDRAGON] & ~bbs[LUNICORN];
    }
    constexpr bitboard_t princess() const
    {
        return bbs[LROOK] & bbs[LBISHOP] & ~bbs[LUNICORN];
    }
    constexpr bitboard_t royal_queen() const
    {
        return bbs[LROOK] & bbs[LDRAGON] & bbs[ROYAL];
    }
    constexpr bitboard_t queen() const
    {
        return bbs[LROOK] & bbs[LDRAGON] & ~bbs[ROYAL];
    }
    constexpr static bitboard_t pmask(int pos)
    {
        return bitboard_t(1) << bitboard_t(pos);
    }
    piece_t get_piece(int pos) const;
    void set_piece(int pos, piece_t p);
    std::array<piece_t, BOARD_SIZE> to_array_board() const;

    // the pieces (both white and black) that attacks a given square
    bitboard_t attacks_to(int pos);
    bool is_under_attack(int pos, int color);
};

static inline bitboard_t white_pawn_attack(int pos)
{
    bitboard_t z = bitboard_t(1) << pos;
    return shift_northwest(z) | shift_northeast(z);
}
static inline bitboard_t black_pawn_attack(int pos)
{
    bitboard_t z = bitboard_t(1) << pos;
    return shift_southwest(z) | shift_southeast(z);
}
bitboard_t knight_attack(int pos);
bitboard_t king_attack(int pos);

#endif // BITBOARD_H
