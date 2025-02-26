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
    void undo();
    void redo();
    bool apply_move(full_move fm);
};



#endif // GAME_H
