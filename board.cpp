#include "board.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>

board::board()
{
    std::fill(this->piece, this->piece + BOARD_SIZE, NO_PIECE);
}

board::board(std::string fen, const int x_size, const int y_size)
{
    std::fill(this->piece, this->piece + BOARD_SIZE, WALL_PIECE);
    char c;
    int x = 0, y = y_size - 1;
    for(int i = 0; i < fen.length(); i++)
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
                    this->set_piece(x, y, KING_UW);
                    break;
                case 'R':
                    this->set_piece(x, y, ROOK_UW);
                    break;
                case 'P':
                    this->set_piece(x, y, PAWN_UW);
                    break;
                case 'k':
                    this->set_piece(x, y, KING_UB);
                    break;
                case 'r':
                    this->set_piece(x, y, ROOK_UB);
                    break;
                case 'p':
                    this->set_piece(x, y, PAWN_UB);
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
                    this->set_piece(x, y, NO_PIECE); x++;
                case '8':
                    this->set_piece(x, y, NO_PIECE); x++;
                case '7':
                    this->set_piece(x, y, NO_PIECE); x++;
                case '6':
                    this->set_piece(x, y, NO_PIECE); x++;
                case '5':
                    this->set_piece(x, y, NO_PIECE); x++;
                case '4':
                    this->set_piece(x, y, NO_PIECE); x++;
                case '3':
                    this->set_piece(x, y, NO_PIECE); x++;
                case '2':
                    this->set_piece(x, y, NO_PIECE); x++;
                case '1':
                    this->set_piece(x, y, NO_PIECE); x++;
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
                    this->set_piece(x, y, (piece_t)c); x++;
                    break;
                default:
                    std::stringstream sstm;
                    sstm << "Bad FEN, unknown piece [" << c << "] in \n" << fen << std::endl;
                    throw std::runtime_error(sstm.str());
            }
        }
    }
}

/**
 * @param x  coordinate x
 * @param y  coordinate y
 * @return the index corresponds to (x,y)
 * The coordinate system works as follows. PGN position "a1" is the position (0,0).
 * In general, "xy" is the position ('x'-'a', y-1)
 */
constexpr int ppos(int x, int y)
{
    return x|(y<<BOARD_BITS);
}

piece_t& board::operator[](int p)
{
    return piece[p];
}

void board::set_piece(int x, int y, piece_t p)
{
    this->piece[ppos(x,y)] = p;
}

board board::replace_piece(int pos, piece_t p) const
{
    board b = *this;
    b.piece[pos] = p;
    return b;
}

board board::move_piece(int from, int to) const
{
    board b = *this;
    b.piece[to] = static_cast<piece_t>(piece_name(piece[from]));
    b.piece[from] = NO_PIECE;
    return b;
}

piece_t board::get_piece(int x, int y) const
{
    return this->piece[ppos(x,y)];
}

std::string board::to_string() const
{
    std::string result = "";
    for (int y = BOARD_LENGTH - 1; y >= 0; y--) 
    {
        for (int x = 0; x < BOARD_LENGTH; x++) 
        {
            switch (this->get_piece(x,y)) 
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
                    result += this->get_piece(x,y);
                    result += " ";
                    break;
            }
        }
        result += "\n";
    }
    return result;
}

std::string board::get_fen() const
{
    std::string result = "";
    for (int y = BOARD_LENGTH - 1; y >= 0; y--) 
    {
        for (int x = 0; x < BOARD_LENGTH; x++) 
        {
            switch(this->get_piece(x,y))
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
                    result += piece_name(this->get_piece(x,y));
                    break;
            }
        }
        result += "/";
    }
    result.pop_back(); // remove the extra '/'
    return result;
}
