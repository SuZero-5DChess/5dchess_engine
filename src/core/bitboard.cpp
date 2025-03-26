#include "bitboard.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "utils.h"
#include "magic.h"

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

boardbits::boardbits(const board& b) : bbs{}
{
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        piece_t p = piece_name(b[i]);
        bitboard_t z = 1;
        z <<= i;
        if(p != NO_PIECE)
        {
            if(get_color(p)==0)
            {
                bbs[boardbits::WHITE] |= z;
            }
            else
            {
                bbs[boardbits::BLACK] |= z;
            }
            switch(to_white(piece_name(p)))
            {
                case KING_W:
                    bbs[boardbits::ROYAL] |= z;
                case COMMON_KING_W:
                    bbs[boardbits::LKING] |= z;
                    break;
                case ROOK_W:
                    bbs[boardbits::LROOK] |= z;
                    break;
                case BISHOP_W:
                    bbs[boardbits::LBISHOP] |= z;
                    break;
                case UNICORN_W:
                    bbs[boardbits::LUNICORN] |= z;
                    break;
                case DRAGON_W:
                    bbs[boardbits::LDRAGON] |= z;
                    break;
                case ROYAL_QUEEN_W:
                    bbs[boardbits::ROYAL] |= z;
                case QUEEN_W:
                    bbs[boardbits::LROOK] |= z;
                    bbs[boardbits::LBISHOP] |= z;
                    bbs[boardbits::LUNICORN] |= z;
                    bbs[boardbits::LDRAGON] |= z;
                    break;
                case PRINCESS_W:
                    bbs[boardbits::LROOK] |= z;
                    bbs[boardbits::LBISHOP] |= z;
                    break;
                case KNIGHT_W:
                    bbs[boardbits::LKNIGHT] |= z;
                    break;
                case BRAWN_W:
                    bbs[boardbits::LRAWN] |= z;
                case PAWN_W:
                    bbs[boardbits::LPAWN] |= z;
                    break;
                default:
                    std::cerr << "bb_full_collection initializer:" << p << "not implemented" << std::endl;
                    break;
            }
        }
    }
}

std::shared_ptr<board> boardbits::to_board() const
{
    std::shared_ptr<board> b = std::make_shared<board>();
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        bitboard_t z = bitboard_t(1) << bitboard_t(i);
        if(z & bbs[WHITE])
        {
            if(z & king())
                (*b)[i] = KING_W;
            else if(z & common_king())
                (*b)[i] = COMMON_KING_W;
            else if(z & queen())
                (*b)[i] = QUEEN_W;
            else if(z & royal_queen())
                (*b)[i] = ROYAL_QUEEN_W;
            else if(z & bishop())
                (*b)[i] = BISHOP_W;
            else if(z & knight())
                (*b)[i] = KNIGHT_W;
            else if(z & rook())
                (*b)[i] = ROOK_W;
            else if(z & pawn())
                (*b)[i] = PAWN_W;
            else if(z & unicorn())
                (*b)[i] = UNICORN_W;
            else if(z & dragon())
                (*b)[i] = DRAGON_W;
            else if(z & brawn())
                (*b)[i] = BRAWN_W;
            else if(z & princess())
                (*b)[i] = PRINCESS_W;
        }
        else if(z & bbs[BLACK])
        {
            if(z & king())
                (*b)[i] = KING_B;
            else if(z & common_king())
                (*b)[i] = COMMON_KING_B;
            else if(z & queen())
                (*b)[i] = QUEEN_B;
            else if(z & royal_queen())
                (*b)[i] = ROYAL_QUEEN_B;
            else if(z & bishop())
                (*b)[i] = BISHOP_B;
            else if(z & knight())
                (*b)[i] = KNIGHT_B;
            else if(z & rook())
                (*b)[i] = ROOK_B;
            else if(z & pawn())
                (*b)[i] = PAWN_B;
            else if(z & unicorn())
                (*b)[i] = UNICORN_B;
            else if(z & dragon())
                (*b)[i] = DRAGON_B;
            else if(z & brawn())
                (*b)[i] = BRAWN_B;
            else if(z & princess())
                (*b)[i] = PRINCESS_B;
        }
        else
        {
            (*b)[i] = NO_PIECE;
        }
    }
    return b;
}

bitboard_t boardbits::attacks_to(int pos)
{
    bitboard_t white = bbs[boardbits::WHITE], black = bbs[boardbits::BLACK];
    bitboard_t pawns = bbs[boardbits::LPAWN], all = white | black;
    bitboard_t white_pawns = pawns & white;
    bitboard_t black_pawns = pawns & black;
    return (white_pawn_attack(pos) & black_pawns)
        |  (black_pawn_attack(pos) & white_pawns)
        |  (king_attack(pos)       & bbs[boardbits::LKING])
        |  (knight_attack(pos)     & bbs[boardbits::LKNIGHT])
        |  (rook_attack(pos, all)  & bbs[boardbits::LROOK])
        |  (bishop_attack(pos, all)& bbs[boardbits::LBISHOP]);
}

bool boardbits::is_under_attack(int pos, int color)
{
    bitboard_t hostile = color ? bbs[boardbits::WHITE] : bbs[boardbits::BLACK];
    bitboard_t pawns = bbs[boardbits::LPAWN];
    bitboard_t white_pawns = pawns & bbs[boardbits::WHITE];
    if(color==0 && white_pawn_attack(pos) & pawns & hostile)
        return true;
    if(color!=0 && black_pawn_attack(pos) & pawns & hostile)
        return true;
    if(king_attack(pos) & bbs[boardbits::LKING] & hostile)
        return true;
    if(knight_attack(pos) & bbs[boardbits::LKNIGHT] & hostile)
        return true;
    bitboard_t all = bbs[boardbits::WHITE] | bbs[boardbits::BLACK];
    if(bishop_attack(pos, all) & bbs[boardbits::LBISHOP] & hostile)
        return true;
    if(rook_attack(pos, all) & bbs[boardbits::LROOK] & hostile)
        return true;
    return false;
}

constexpr std::array<bitboard_t, BOARD_SIZE> knight_attack_data = generate_array(std::make_index_sequence<BOARD_SIZE>{}, [](size_t pos) -> bitboard_t
{
    bitboard_t z = bitboard_t(1) << pos, bb = 0;
    bb |= shift_north(shift_northwest(z));
    bb |= shift_west(shift_northwest(z));
    bb |= shift_north(shift_northeast(z));
    bb |= shift_east(shift_northeast(z));
    bb |= shift_south(shift_southwest(z));
    bb |= shift_west(shift_southwest(z));
    bb |= shift_south(shift_southeast(z));
    bb |= shift_east(shift_southeast(z));
    return bb;
});

constexpr std::array<bitboard_t, BOARD_SIZE> king_attack_data = generate_array(std::make_index_sequence<BOARD_SIZE>{}, [](size_t pos) -> bitboard_t
{
    bitboard_t z = bitboard_t(1) << pos, bb = 0;
    bb |= shift_north(z);
    bb |= shift_south(z);
    bb |= shift_west(z);
    bb |= shift_east(z);
    bb |= shift_northwest(z);
    bb |= shift_northeast(z);
    bb |= shift_southwest(z);
    bb |= shift_southeast(z);
    return bb;
});
                                                                        
bitboard_t knight_attack(int pos)
{
    return knight_attack_data[pos];
}

bitboard_t king_attack(int pos)
{
    return king_attack_data[pos];
}
