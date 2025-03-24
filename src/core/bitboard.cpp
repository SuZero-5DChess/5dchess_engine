#include "bitboard.h"
#include <iostream>
#include <iomanip>
#include <sstream>

std::string bitboard_to_string(bitboard_t bb)
{
    std::ostringstream ss;
    ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << bb << std::endl;
    for (int y = BOARD_LENGTH - 1; y >= 0; y--)
    {
        for (int x = 0; x < BOARD_LENGTH; x++)
        {
            ss << ((bb & (static_cast<bitboard_t>(1)<<board::ppos(x, y))) ? "1 " : ". ");
        }
        ss << "\n";
    }
    return ss.str();
}

bb_full_collection::bb_full_collection(const board& b)
{
    white = 0;
    black = 0;
    royal = 0;
    king = 0;
    rook = 0;
    bishop = 0;
    unicorn = 0;
    dragon = 0;
    queen = 0;
    knight = 0;
    pawn = 0;
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        piece_t p = b[i];
        bitboard_t z = 1;
        z <<= i;
        if(p != NO_PIECE)
        {
            if(get_color(p)==0)
            {
                white |= z;
            }
            else
            {
                black |= z;
            }
            switch(to_white(piece_name(p)))
            {
                case KING_W:
                    royal |= z;
                case COMMON_KING_W:
                    king |= z;
                    break;
                case ROOK_W:
                    rook |= z;
                    break;
                case BISHOP_W:
                    bishop |= z;
                    break;
                case UNICORN_W:
                    unicorn |= z;
                    break;
                case DRAGON_W:
                    dragon |= z;
                    break;
                case ROYAL_QUEEN_W:
                    royal |= z;
                case QUEEN_W:
                    queen |= z;
                    break;
                case KNIGHT_W:
                    knight |= z;
                    break;
                case PAWN_W:
                    pawn |= z;
                    break;
                
                default:
                    std::cerr << "bb_full_collection initializer:" << p << "not implemented" << std::endl;
                    break;
            }
        }
    }
}
