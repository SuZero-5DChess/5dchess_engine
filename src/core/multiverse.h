//
//  board_2d.h
//  5dchess_engine
//
//  Created by ftxi on 2024/12/5.
//

#ifndef MULTIVERSE_H
#define MULTIVERSE_H

#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <map>
#include <memory>
#include "board.h"
#include "vec4.h"



/*
 Behavior of copying a multiverse object is just copy the vector of vectors of pointers to the boards. It does not perform deep-copy of a board object. (Which is expected.)
 */
class multiverse
{
private:
    std::vector<std::vector<std::shared_ptr<board>>> boards;
    
    // move generation for each pieces
    enum class axesmode {ORTHOGONAL, DIAGONAL, BOTH};
    
    template<bool C, axesmode TL, axesmode XY>
    void gen_compound_moves(vec4 p, std::map<vec4, bitboard_t>& result) const;
    

    template<piece_t P, bool C>
    bitboard_t gen_physical_moves_impl(vec4 p) const;

    template<piece_t P, bool C>
    std::map<vec4, bitboard_t> gen_superphysical_moves_impl(vec4 p) const;

    template<bool C>
    std::map<vec4, bitboard_t> gen_purely_sp_rook_moves(vec4 p) const;
    
    template<bool C>
    std::map<vec4, bitboard_t> gen_purely_sp_bishop_moves(vec4 p0) const;
    
    template<bool C>
    std::map<vec4, bitboard_t> gen_purely_sp_knight_moves(vec4 p0) const;
public:
    // the following data are derivated from boards:
    int l_min, l_max;
    std::vector<int> timeline_start, timeline_end;

    multiverse(const std::string& input, int size_x = BOARD_LENGTH, int size_y = BOARD_LENGTH);
    multiverse(const multiverse& other);
    multiverse& operator=(const multiverse& other);
    
    std::shared_ptr<board> get_board(int l, int t, int c) const;
    void insert_board(int l, int t, int c, const std::shared_ptr<board>& b_ptr);
    void append_board(int l, const std::shared_ptr<board>& b_ptr);
    std::vector<std::tuple<int,int,int,std::string>> get_boards() const;
    std::string to_string() const;
    bool inbound(vec4 a, int color) const;
    piece_t get_piece(vec4 a, int color) const;
    bool get_umove_flag(vec4 a, int color) const;
    
    /*
     `number_activated` is max(abs(white's activated lines), abs(black's activated lines)).
     */
    int number_activated() const;
    /*
     This helper function returns (present_t, present_c)
     where: present_t is the time of present in L,T coordinate
            present_c is either 0 (for white) or 1 (for black)
     */
    std::tuple<int,int> get_present() const;
    bool is_active(int l) const;
    
    template<bool C>
    std::map<vec4, bitboard_t> gen_superphysical_moves(vec4 p) const;

    template<bool C>
    std::map<vec4, bitboard_t> gen_moves(vec4 p) const;
    
    std::vector<vec4> gen_piece_move(vec4 p, int board_color) const;
    
    
    
    /*
     The following static functions describe the correspondence between two coordinate systems: L,T and u,v
     
    l_to_u make use of the bijection from integers to non-negative integers:
    x -> ~(x>>1)
     */
    constexpr static int l_to_u(int l)
    {
        if(l >= 0)
            return l << 1;
        else
            return ~(l << 1);
    }

    constexpr static int tc_to_v(int t, int c)
    {
        return t << 1 | c;
    }

    constexpr static int u_to_l(int u)
    {
        if(u & 1)
            return ~(u >> 1);
        else
            return u >> 1;
    }

    constexpr static std::tuple<int, int> v_to_tc(int v)
    {
        return std::tuple<int, int>(v >> 1, v & 1);
    }
};



#endif /* MULTIVERSE_H */
