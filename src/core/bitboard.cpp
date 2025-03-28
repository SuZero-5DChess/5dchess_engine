#include "bitboard.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "utils.h"
#include "magic.h"

std::string bb_to_string(bitboard_t bb)
{
    std::ostringstream ss;
    ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << bb << std::endl;
    for (int y = BOARD_LENGTH - 1; y >= 0; y--)
    {
        for (int x = 0; x < BOARD_LENGTH; x++)
        {
            ss << ((bb & (static_cast<bitboard_t>(1)<<ppos(x, y))) ? "1 " : ". ");
        }
        ss << "\n";
    }
    return ss.str();
}


std::vector<int> marked_pos(bitboard_t b)
{
    std::vector<int> result;
    while(b)
    {
        int n = bb_get_pos(b);
        result.push_back(n);
        b &= ~(bitboard_t(1) << n);
    }
    return result;
}

boardbits::boardbits(std::array<piece_t, BOARD_SIZE> b) : bbs{}
{
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        piece_t p = piece_name(b[i]);
        set_piece(i, p);
    }
}

piece_t boardbits::get_piece(int pos) const
{
    piece_t piece;
    bitboard_t z = pmask(pos);
    if(z & bbs[WHITE])
    {
        if(z & king())
            piece = KING_W;
        else if(z & common_king())
            piece = COMMON_KING_W;
        else if(z & queen())
            piece = QUEEN_W;
        else if(z & royal_queen())
            piece = ROYAL_QUEEN_W;
        else if(z & bishop())
            piece = BISHOP_W;
        else if(z & knight())
            piece = KNIGHT_W;
        else if(z & rook())
            piece = ROOK_W;
        else if(z & pawn())
            piece = PAWN_W;
        else if(z & unicorn())
            piece = UNICORN_W;
        else if(z & dragon())
            piece = DRAGON_W;
        else if(z & brawn())
            piece = BRAWN_W;
        else if(z & princess())
            piece = PRINCESS_W;
    }
    else if(z & bbs[BLACK])
    {
        if(z & king())
            piece = KING_B;
        else if(z & common_king())
            piece = COMMON_KING_B;
        else if(z & queen())
            piece = QUEEN_B;
        else if(z & royal_queen())
            piece = ROYAL_QUEEN_B;
        else if(z & bishop())
            piece = BISHOP_B;
        else if(z & knight())
            piece = KNIGHT_B;
        else if(z & rook())
            piece = ROOK_B;
        else if(z & pawn())
            piece = PAWN_B;
        else if(z & unicorn())
            piece = UNICORN_B;
        else if(z & dragon())
            piece = DRAGON_B;
        else if(z & brawn())
            piece = BRAWN_B;
        else if(z & princess())
            piece = PRINCESS_B;
    }
    else
    {
        piece = NO_PIECE;
    }
    return piece;
}

void boardbits::set_piece(int pos, piece_t p)
{
    bitboard_t z = pmask(pos);
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

std::array<piece_t, BOARD_SIZE> boardbits::to_array_board() const
{
    std::array<piece_t, BOARD_SIZE> b;
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        b[i] = get_piece(i);
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
