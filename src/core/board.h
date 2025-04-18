//
//  board_2d.h
//  5dchess_backend
//
//  Created by ftxi on 2024/4/24.
//

#ifndef board_h
#define board_h

#include <iostream>
#include <array>
#include <string>
#include "piece.h"
#include "bitboard.h"

class array_board;

/*
The bitboards collection associated with a board.
 */
class board
{
    enum bitboard_indices {
        WHITE, BLACK, ROYAL, //flags
        LKING, LKNIGHT, LPAWN, LRAWN, //jumping pieces
        LROOK, LBISHOP, LUNICORN, LDRAGON, //sliding pieces
        BBS_INDICES_COUNT
    };
    std::array<bitboard_t, BBS_INDICES_COUNT> bbs;
    bitboard_t umove_mask;

public:
    board(std::string fen, const int x_size = BOARD_LENGTH, const int y_size = BOARD_LENGTH);
    // inline getter functions
    constexpr bitboard_t umove() const { return umove_mask; }

    constexpr bitboard_t white() const { return bbs[WHITE]; }
    constexpr bitboard_t black() const { return bbs[BLACK]; }
    
    constexpr bitboard_t royal() const { return bbs[ROYAL]; }

    constexpr bitboard_t lking() const { return bbs[LKING]; }
    constexpr bitboard_t lknight() const { return bbs[LKNIGHT]; }
    constexpr bitboard_t lpawn() const { return bbs[LPAWN]; }
    constexpr bitboard_t lrawn() const { return bbs[LRAWN]; }

    constexpr bitboard_t lrook() const { return bbs[LROOK]; }
    constexpr bitboard_t lbishop() const { return bbs[LBISHOP]; }
    constexpr bitboard_t lunicorn() const { return bbs[LUNICORN]; }
    constexpr bitboard_t ldragon() const { return bbs[LDRAGON]; }

    constexpr bitboard_t king() const { return bbs[LKING] & bbs[ROYAL]; }
    constexpr bitboard_t common_king() const { return bbs[LKING] & ~bbs[ROYAL]; }
    constexpr bitboard_t knight() const { return bbs[LKNIGHT]; }
    constexpr bitboard_t pawn() const { return bbs[LPAWN] & ~bbs[LRAWN]; }
    constexpr bitboard_t brawn() const { return bbs[LPAWN] & bbs[LRAWN]; }
    constexpr bitboard_t rook() const { return bbs[LROOK] & ~bbs[LBISHOP]; }
    constexpr bitboard_t bishop() const { return bbs[LBISHOP] & ~bbs[LROOK]; }
    constexpr bitboard_t unicorn() const { return bbs[LUNICORN] & ~bbs[LDRAGON]; }
    constexpr bitboard_t dragon() const { return bbs[LDRAGON] & ~bbs[LUNICORN]; }
    constexpr bitboard_t princess() const { return bbs[LROOK] & bbs[LBISHOP] & ~bbs[LUNICORN]; }
    constexpr bitboard_t royal_queen() const { return bbs[LROOK] & bbs[LDRAGON] & bbs[ROYAL]; }
    constexpr bitboard_t queen() const { return bbs[LROOK] & bbs[LDRAGON] & ~bbs[ROYAL]; }
    
    // indirect getter functions
    constexpr bitboard_t occupied() const { return bbs[WHITE] | bbs[BLACK]; }
    
    template<bool C>
    constexpr bitboard_t friendly() const
    {
        if constexpr (C)
        {
            return bbs[BLACK];
        }
        else
        {
            return bbs[WHITE];
        }
    }
    
    template<bool C>
    constexpr bitboard_t hostile() const
    {
        if constexpr (C)
        {
            return bbs[WHITE];
        }
        else
        {
            return bbs[BLACK];
        }
    }
    
    // modifications
    piece_t get_piece(int pos) const;
    void set_piece(int pos, piece_t p);
    std::shared_ptr<board> replace_piece(int pos, piece_t p) const;
    std::shared_ptr<board> move_piece(int from, int to) const;
    array_board to_array_board() const;
    std::string to_string() const;
    std::string get_fen() const;

    // all the pieces (both white and black) that attacks a given square
    bitboard_t attacks_to(int pos) const;
	// pieces hositile to `color` that attacks a given square
    bool is_under_attack(int pos, int color) const;
};


class array_board {
private:
    std::array<piece_t, BOARD_SIZE> piece;
public:
    array_board(): piece{}{}
    array_board(std::string fen, const int x_size = BOARD_LENGTH, const int y_size = BOARD_LENGTH);
    
    // the getter method
    piece_t get_piece(int p) const;
    //const piece_t& operator[](int p) const;
    // the setter method, which is deprecated. Use replace_piece or move_piece whenever possible
    void set_piece(int pos, piece_t p);

    // these methods return the pointer to a new board object so that there is no side effect
    std::shared_ptr<array_board> replace_piece(int pos, piece_t p) const;
    std::shared_ptr<array_board> move_piece(int from, int to) const;

    std::string to_string() const;
    std::string get_fen() const;
#ifdef SHOW_BOARD_DESTRUCTION
    ~board()
    {
        std::cerr << "Board destroyed: " << get_fen() << std::endl;
    }
#endif
};


#endif /* board_h */
