//
//  board_2d.h
//  5dchess_backend
//
//  Created by ftxi on 2024/4/24.
//

#ifndef board_2d_h
#define board_2d_h

#include <string>

#define BOARD2D_BITS 3
#define BOARD2D_LENGTH (1<<BOARD2D_BITS)
//the cap of length and height of the board <- 8
#define BOARD2D_SIZE (1<<BOARD2D_BITS<<BOARD2D_BITS)
//size of the board <- 64 (normal chess board)

typedef enum : char { //syntax of c23. Use lesser memory
    NO_PIECE = 0,
    //empty
    WALL_PIECE = 1,
    //in case this place is not in a active board
    KING_UW, ROOK_UW, PAWN_UW,
    //unmoved standard pieces for white
    KING_UB, ROOK_UB, PAWN_UB,
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

class board2d {
private:
    piece_t piece[BOARD2D_SIZE];
public:
    board2d();
    board2d(std::string fen, const int x_size = BOARD2D_LENGTH, const int y_size = BOARD2D_LENGTH);
    void set_piece(int x, int y, piece_t p);
    piece_t get_piece(int x, int y) const;
    std::string to_string() const;
};


#endif /* board_2d_h */
