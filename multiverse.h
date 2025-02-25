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
#include <ranges>
#include "board.h"
#include "vec4.h"

using std::tuple;
using std::pair;
using std::vector;
using std::map;
using std::string;
using std::shared_ptr;
namespace ranges = std::ranges;

/*
 Behavior of copying a multiverse object is just copy the vector of vectors of pointers to the boards. It does not perform deep-copy of a board object. (Which is expected.)
 */
class multiverse {
private:
    vector<vector<shared_ptr<board>>> boards;
public:
    // the following data are derivated from boards:
    int l_min, l_max;
    vector<int> timeline_start, timeline_end;
    
    //map<string, string> metadata;

    multiverse(const std::string& input);
    
    shared_ptr<board> get_board(int l, int t, int c) const;
    void insert_board(int l, int t, int c, const shared_ptr<board>& b_ptr);
    void append_board(int l, const shared_ptr<board>& b_ptr);
    vector<tuple<int,int,int,string>> get_boards() const;
    string to_string() const;
    bool inbound(vec4 a, int color) const;
    piece_t get_piece(vec4 a, int color) const;

    // This helper function returns (number_activated, present_t, current_player)
    // where: number_activated is max(abs(white's activated lines), abs(black's activated lines))
    //        present_t is the time of present in L,T coordinate
    //        present_c is either 0 (for white) or 1 (for black)
    std::tuple<int,int,int> get_present() const;

    vector<vec4> gen_piece_move(const vec4& p, int board_color) const;
    
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
