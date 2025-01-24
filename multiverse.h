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
#include <map>
#include <memory>
#include <ranges>
#include "board.h"
#include "vec4.h"

using std::tuple;
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
    // the following data are derivated from boards:
    int l_min, l_max;
    vector<int> timeline_start, timeline_end;
    int number_activated, present;
    
    bool check_multiverse_shape();
public:
    map<string, string> metadata;

    multiverse(const std::string& input);
    
    shared_ptr<board> get_board(int l, int t, int c) const;
    vector<tuple<int,int,int,string>> get_boards() const;
    string to_string() const;
    bool inbound(vec4 a, int color) const;
    piece_t get_piece(vec4 a, int color) const;
    vector<vec4> gen_piece_move(const vec4& p, int board_color) const;
};

#endif /* MULTIVERSE_H */
