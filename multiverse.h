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
#include <deque>
#include <map>
#include <memory>
#include "board.h"

using std::tuple;
using std::vector;
using std::map;
using std::string;
using std::shared_ptr;

class multiverse {
private:
    vector<vector<shared_ptr<board>>> boards;
public:
    map<string, string> metadata;

    multiverse(const std::string& input);
    // virtual ~multiverse();
    
    shared_ptr<board> get_board(int t, int l, int c) const;
    vector<tuple<int,int,int,string>> get_boards() const;
    string to_string() const;
};

#endif /* MULTIVERSE_H */
