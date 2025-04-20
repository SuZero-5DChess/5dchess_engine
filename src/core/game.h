#ifndef GAME_H
#define GAME_H

#include <vector>
#include <tuple>
#include <string>
#include <map>
#include <set>
#include "state.h"

class game
{
    std::vector<state> cached_states;
    std::vector<state>::iterator now;
public:
    std::map<std::string, std::string> metadata;
    
    game(std::string str);
    state get_current_state() const;
    std::tuple<int,int> get_current_present() const;
    std::vector<std::tuple<int,int,int,std::string>> get_current_boards() const;
    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> get_current_timeline_status() const;
    std::vector<vec4> gen_move_if_playable(vec4 p);
    
    match_status_t get_match_status() const;
    std::vector<vec4> get_movable_pieces() const;
    
    bool is_playable(vec4 p) const;
    bool can_undo() const;
    bool can_redo() const;
    bool can_submit() const;
    void undo();
    void redo();
    bool apply_move(full_move fm);
    bool apply_indicator_move(full_move fm);
    std::vector<std::pair<vec4,vec4>> get_current_checks() const;
    
    std::tuple<int, int> get_board_size() const;
};



#endif // GAME_H
