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

class board {
private:
    std::array<piece_t, BOARD_SIZE> piece;
    boardbits bits;
public:
    board(): piece{}{}
    board(std::string fen, const int x_size = BOARD_LENGTH, const int y_size = BOARD_LENGTH);
    
    // the getter method
    piece_t get_piece(int p) const;
    //const piece_t& operator[](int p) const;
    // the setter method, which is deprecated. Use replace_piece or move_piece whenever possible
    void set_piece(int x, int y, piece_t p);

    // these methods return the pointer to a new board object so that there is no side effect
    std::shared_ptr<board> replace_piece(int pos, piece_t p) const;
    std::shared_ptr<board> move_piece(int from, int to) const;

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
