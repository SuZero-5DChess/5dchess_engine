#ifndef PIECE_H
#define PIECE_H

#define BOARD_BITS 3
#define BOARD_LENGTH (1<<BOARD_BITS)
//the cap of length and height of the board <- 8
#define BOARD_SIZE (1<<BOARD_BITS<<BOARD_BITS)
//size of the board <- 64 (normal chess board)

typedef enum : unsigned char { //syntax of c23/c++11. Use lesser memory
    NO_PIECE = 0,
    //empty
    WALL_PIECE = 1,
    //in case this place is not in a active board
    KING_UW = 'K'| 0x80, ROOK_UW = 'R'|0x80, PAWN_UW = 'P'|0x80,
    //unmoved standard pieces for white
    KING_UB = 'k'| 0x80, ROOK_UB = 'r'|0x80, PAWN_UB = 'p'|0x80,
    //unmoved standard pieces for black
    
    KING_W = 'K', QUEEN_W = 'Q', BISHOP_W = 'B',
    KNIGHT_W = 'N', ROOK_W = 'R', PAWN_W = 'P',
    //standard pieces for white
    
    UNICORN_W = 'U', DRAGON_W = 'D', BRAWN_W = 'W',
    PRINCESS_W = 'S', ROYAL_QUEEN_W = 'Y', COMMON_KING_W = 'C',
    //nonstandard pieces for white
    KING_B = 'k', QUEEN_B = 'q', BISHOP_B = 'b',
    KNIGHT_B = 'n', ROOK_B = 'r', PAWN_B = 'p',
    //standard pieces for black
    
    UNICORN_B = 'u', DRAGON_B = 'd', BRAWN_B = 'w',
    PRINCESS_B = 's', ROYAL_QUEEN_B = 'y', COMMON_KING_B = 'c',
    //nonstandard pieces for black
} piece_t; //totally: 30 pieces + 2 non-pieces

// remove the unmoved signature for pawn/rook/king
constexpr piece_t piece_name(piece_t p)
{
    return static_cast<piece_t>(p & 0x7f);
}

// convert to white piece by capitalizing letters
constexpr piece_t to_white(piece_t p)
{
    return static_cast<piece_t>(p & ~0x20);
}

constexpr piece_t to_black(piece_t p)
{
    return static_cast<piece_t>(p | 0x20);
}

constexpr piece_t switch_color(piece_t p)
{
    return static_cast<piece_t>(p ^ 0x20);
}

// detect if the letter is in lower case
constexpr bool get_color(piece_t p)
{
    return p & 0x20;
}

/**
 * @param x  coordinate x
 * @param y  coordinate y
 * @return the index corresponds to (x,y)
 * The coordinate system works as follows. PGN position "a1" is the position (0,0).
 * In general, "xy" is the position ('x'-'a', y-1)
 */
constexpr static int ppos(int x, int y)
{
    return x|(y<<BOARD_BITS);
}

#endif //PIECE_H
