#include "board.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>

#include "magic.h"

board::board(std::string fen, int size_x, int size_y) : bbs{}, umove_mask{0}
{
    array_board arrb(fen, size_x, size_y);
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        piece_t p0 = arrb.get_piece(i);
        piece_t p = piece_name(p0);
        set_piece(i, p);
        if(piece_umove_flag(p0))
        {
            umove_mask |= pmask(i);
        }
    }
}

piece_t board::get_piece(int pos) const
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
        else if(z & wall())
            piece = WALL_PIECE;
        else
            throw std::runtime_error("get_piece: unknown piece\n");
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
        else
            throw std::runtime_error("board::get_piece: unknown piece\n");
    }
    else
    {
        piece = NO_PIECE;
    }
    return piece;
}

void board::set_piece(int pos, piece_t p)
{
    bitboard_t z = pmask(pos);
    umove_mask &= ~z;
    for(int i = 0; i < BBS_INDICES_COUNT; i++)
    {
        bbs[i] &= ~z;
    }
    if(p == NO_PIECE)
    {
        //pass
    }
    else if(p == WALL_PIECE)
    {
        bbs[board::WHITE] |= z;
        bbs[board::BLACK] |= z;
    }
    else
    {
        if(piece_color(p)==0)
        {
            bbs[board::WHITE] |= z;
        }
        else
        {
            bbs[board::BLACK] |= z;
        }
        switch(to_white(piece_name(p)))
        {
            case KING_W:
                bbs[board::ROYAL] |= z;
                [[fallthrough]];
            case COMMON_KING_W:
                bbs[board::LKING] |= z;
                break;
            case ROOK_W:
                bbs[board::LROOK] |= z;
                break;
            case BISHOP_W:
                bbs[board::LBISHOP] |= z;
                break;
            case UNICORN_W:
                bbs[board::LUNICORN] |= z;
                break;
            case DRAGON_W:
                bbs[board::LDRAGON] |= z;
                break;
            case ROYAL_QUEEN_W:
                bbs[board::ROYAL] |= z;
                [[fallthrough]];
            case QUEEN_W:
                bbs[board::LROOK] |= z;
                bbs[board::LBISHOP] |= z;
                bbs[board::LUNICORN] |= z;
                bbs[board::LDRAGON] |= z;
                break;
            case PRINCESS_W:
                bbs[board::LROOK] |= z;
                bbs[board::LBISHOP] |= z;
                break;
            case KNIGHT_W:
                bbs[board::LKNIGHT] |= z;
                break;
            case BRAWN_W:
                bbs[board::LRAWN] |= z;
                [[fallthrough]];
            case PAWN_W:
                bbs[board::LPAWN] |= z;
                break;
            default:
                std::ostringstream oss;
                oss << "board::set_piece:" << p << "not implemented" << std::endl;
                throw std::runtime_error(oss.str());
                break;
        }
    }
}

std::shared_ptr<board> board::replace_piece(int pos, piece_t p) const
{
    std::shared_ptr<board> b_ptr = std::make_shared<board>(*this);
    b_ptr->set_piece(pos, p);
    return b_ptr;
}

std::shared_ptr<board> board::move_piece(int from, int to) const
{
    std::shared_ptr<board> b_ptr = std::make_shared<board>(*this);
    b_ptr->set_piece(to, get_piece(from));
    b_ptr->set_piece(from, NO_PIECE);
    return b_ptr;
}

array_board board::to_array_board() const
{
    array_board arrb;
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        arrb.set_piece(i, get_piece(i));
    }
    return arrb;
}

std::string board::to_string() const
{
    return to_array_board().to_string();
}

template<bool SHOW_UMOVE>
std::string board::get_fen() const
{
    std::string result = "/";
    for (int y = BOARD_LENGTH - 1; y >= 0; y--)
    {
        for (int x = 0; x < BOARD_LENGTH; x++)
        {
            switch(this->get_piece(ppos(x,y)))
            {
                case NO_PIECE:
                    if(isdigit(result.back()))
                    {
                        result.back() += 1;
                    }
                    else
                    {
                        result += '1';
                    }
                    break;
                case WALL_PIECE:
                    continue;
                default:
                    result += get_piece(ppos(x,y));
                    if constexpr (SHOW_UMOVE)
                    {
                        if(umove() & pmask(ppos(x, y)))
                        {
                            result += "*";
                        }
                    }
                    break;
            }
        }
        if(result.back() != '/')
        {
            result += "/";
        }
    }
    result.erase(result.begin());
    result.pop_back(); // remove the extra '/'
    return result;
}

bitboard_t board::attacks_to(int pos) const
{
    bitboard_t w = white(), b = black();
    bitboard_t pawns = lpawn(), all = w | b;
    bitboard_t white_pawns = pawns & w;
    bitboard_t black_pawns = pawns & b;
    return (white_pawn_attack(pos) & black_pawns)
        |  (black_pawn_attack(pos) & white_pawns)
        |  (king_attack(pos)       & lking())
        |  (knight_attack(pos)     & lknight())
        |  (rook_attack(pos, all)  & lrook())
        |  (bishop_attack(pos, all)& lbishop());
}

bool board::is_under_attack(int pos, int color) const
{
    bitboard_t hostile = color ? white() : black();
    bitboard_t pawns = lpawn();
    if(color == 0 && white_pawn_attack(pos) & pawns & hostile)
        return true;
    if(color != 0 && black_pawn_attack(pos) & pawns & hostile)
        return true;
    if(king_attack(pos) & lking() & hostile)
        return true;
    if(knight_attack(pos) & lknight() & hostile)
        return true;
    bitboard_t all = white() | black();
    if(bishop_attack(pos, all) & lbishop() & hostile)
        return true;
    if(rook_attack(pos, all) & lrook() & hostile)
        return true;
    return false;
}


/***************************************************** */

array_board::array_board(std::string fen, const int x_size, const int y_size)
{
    piece.fill(WALL_PIECE);
    char c;
    int x = 0, y = y_size - 1;
    for(size_t i = 0; i < fen.length(); i++)
    {
        c = fen[i];
        if(i+1 < fen.length() && fen[i+1] == '*')
        {
            if(x >= x_size) // check overflow
            {
                std::stringstream sstm;
                sstm << "Bad FEN, x=" << x << " overflows in \n" << fen << std::endl;
                throw std::runtime_error(sstm.str());
            }
            switch(c)
            {
                case 'K':
                    this->set_piece(ppos(x,y), KING_UW);
                    break;
                case 'R':
                    this->set_piece(ppos(x,y), ROOK_UW);
                    break;
                case 'P':
                    this->set_piece(ppos(x,y), PAWN_UW);
                    break;
                case 'W':
                    this->set_piece(ppos(x, y), BRAWN_UW);
                    break;
                case 'k':
                    this->set_piece(ppos(x,y), KING_UB);
                    break;
                case 'r':
                    this->set_piece(ppos(x,y), ROOK_UB);
                    break;
                case 'p':
                    this->set_piece(ppos(x,y), PAWN_UB);
                    break;
                case 'w':
                    this->set_piece(ppos(x, y), BRAWN_UB);
                    break;
                default:
                    std::stringstream sstm;
                    sstm << "Bad FEN: undefined behavior: * after [" << c << "] in \n" << fen << std::endl;
                    throw std::runtime_error(sstm.str());
            }
            i++; //so that this loop increment i by 2, skipping c and the following *
            x++;
        }
        else if(c == '/')
        {
            y--;
            if(y < 0)
            {
                std::stringstream sstm;
                sstm << "Bad FEN, y=" << y << " overflows in \n" << fen << std::endl;
                throw std::runtime_error(sstm.str());
            }
            x = 0;
        }
        else if(isspace(c))
        {
            //pass
        }
        else
        {
            int shift = 1;
            if(isdigit(c))
                shift = c - '0';
            if(x + shift > x_size) // check overflow
            {
                std::stringstream sstm;
                sstm << "Bad FEN, x=" << x + shift << " overflows in \n" << fen << std::endl;
                throw std::runtime_error(sstm.str());
            }
            switch(c)
            {
                case '9':
                    this->set_piece(ppos(x,y), NO_PIECE); x++;
                    [[fallthrough]];
                case '8':
                    this->set_piece(ppos(x,y), NO_PIECE); x++;
                    [[fallthrough]];
                case '7':
                    this->set_piece(ppos(x,y), NO_PIECE); x++;
                    [[fallthrough]];
                case '6':
                    this->set_piece(ppos(x,y), NO_PIECE); x++;
                    [[fallthrough]];
                case '5':
                    this->set_piece(ppos(x,y), NO_PIECE); x++;
                    [[fallthrough]];
                case '4':
                    this->set_piece(ppos(x,y), NO_PIECE); x++;
                    [[fallthrough]];
                case '3':
                    this->set_piece(ppos(x,y), NO_PIECE); x++;
                    [[fallthrough]];
                case '2':
                    this->set_piece(ppos(x,y), NO_PIECE); x++;
                    [[fallthrough]];
                case '1':
                    this->set_piece(ppos(x,y), NO_PIECE); x++;
                    [[fallthrough]];
                case '0':
                    break;
                case 'K':
                case 'Q':
                case 'B':
                case 'N':
                case 'R':
                case 'P':
                case 'U':
                case 'D':
                case 'W':
                case 'S':
                case 'Y':
                case 'C':
                case 'k':
                case 'q':
                case 'b':
                case 'n':
                case 'r':
                case 'p':
                case 'u':
                case 'd':
                case 'w':
                case 's':
                case 'y':
                case 'c':
                    this->set_piece(ppos(x,y), (piece_t)c); x++;
                    break;
                default:
                    std::stringstream sstm;
                    sstm << "Bad FEN, unknown piece [" << c << "] in \n" << fen << std::endl;
                    throw std::runtime_error(sstm.str());
            }
        }
    }
}



piece_t array_board::get_piece(int p) const
{
    return piece[p];
}

//const piece_t& board::operator[](int p) const
//{
//    return piece[p];
//}
//
void array_board::set_piece(int pos, piece_t p)
{
    this->piece[pos] = p;
}

std::shared_ptr<array_board> array_board::replace_piece(int pos, piece_t p) const
{
    std::shared_ptr<array_board> b_ptr = std::make_shared<array_board>(*this);
    b_ptr->piece[pos] = p;
    return b_ptr;
}

std::shared_ptr<array_board> array_board::move_piece(int from, int to) const
{
    std::shared_ptr<array_board> b_ptr = std::make_shared<array_board>(*this);
    b_ptr->piece[to] = static_cast<piece_t>(piece_name(piece[from]));
    b_ptr->piece[from] = NO_PIECE;
    return b_ptr;
}
//
//piece_t board::get_piece(int x, int y) const
//{
//    return this->piece[ppos(x,y)];
//}

std::string array_board::to_string() const
{
    std::string result = "";
    for (int y = BOARD_LENGTH - 1; y >= 0; y--) 
    {
        for (int x = 0; x < BOARD_LENGTH; x++) 
        {
            switch (this->get_piece(ppos(x, y)))
            {
                case NO_PIECE:
                    result += ". ";
                    break;
                case WALL_PIECE:
                    result += "##";
                    break;
                case KING_UW:
                    result += "K'";
                    break;
                case ROOK_UW:
                    result += "R'";
                    break;
                case PAWN_UW:
                    result += "P'";
                    break;
                case KING_UB:
                    result += "k'";
                    break;
                case ROOK_UB:
                    result += "r'";
                    break;
                case PAWN_UB:
                    result += "p'";
                    break;
                default:
                    result += this->get_piece(ppos(x,y));
                    result += " ";
                    break;
            }
        }
        result += "\n";
    }
    return result;
}

std::string array_board::get_fen() const
{
    std::string result = "/";
    for (int y = BOARD_LENGTH - 1; y >= 0; y--)
    {
        for (int x = 0; x < BOARD_LENGTH; x++) 
        {
            switch(this->get_piece(ppos(x,y)))
            {
                case NO_PIECE:
                    if(isdigit(result.back()))
                    {
                        result.back() += 1;
                    }
                    else
                    {
                        result += '1';
                    }
                    break;
                case WALL_PIECE:
                    continue;
                default:
                    result += piece_name(this->get_piece(ppos(x,y)));
                    break;
            }
        }
        if(result.back() != '/')
        {
            result += "/";
        }
    }
    result.erase(result.begin());
    result.pop_back(); // remove the extra '/'
    return result;
}

template std::string board::get_fen<true>() const;
template std::string board::get_fen<false>() const;
