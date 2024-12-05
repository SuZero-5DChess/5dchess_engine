#include "board_2d.h"
#include <algorithm>

board2d::board2d()
{
    std::fill(this->piece, this->piece + BOARD2D_SIZE, NO_PIECE);
}

/**
 * @param x  coordinate x
 * @param y  coordinate y
 * @return the index corresponds to (x,y)
 * The coordinate system works as follows. PGN position "a1" is the position (0,0).
 * In general, "xy" is the position ('x'-'a', y-1)
 */
static inline int ppos(int x, int y)
{
    return x|(y<<BOARD2D_BITS);
}

void board2d::set_piece(int x, int y, piece_t p)
{
    this->piece[ppos(x,y)] = p;
}

piece_t board2d::get_piece(int x, int y) const
{
    return piece[ppos(x,y)];
}

