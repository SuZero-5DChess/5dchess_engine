#ifndef GAME_H
#define GAME_H

#include <vector>
#include <tuple>
#include <string>
#include <map>
#include "state.h"

class game {
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
    
    bool is_playable(vec4 p);
    bool can_undo() const;
    bool can_redo() const;
    bool can_submit() const;
    void undo();
    void redo();
    bool apply_move(full_move fm);
};



#endif // GAME_H
