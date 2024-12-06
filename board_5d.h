//
//  board_2d.h
//  5dchess_engine
//
//  Created by ftxi on 2024/12/5.
//

#ifndef board_5d_h
#define board_5d_h

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <memory>
#include "board_2d.h"

using std::deque;
using std::vector;
using std::map;
using std::string;
using std::shared_ptr;

class board5d {
private:
    vector<vector<shared_ptr<board2d>>> boards;
public:
    map<string, string> metadata;

    board5d(const std::string& input);
    // virtual ~board5d();
    
    shared_ptr<board2d> get_board(int t, int l, int c) const;
    string to_string() const;
};

#endif
