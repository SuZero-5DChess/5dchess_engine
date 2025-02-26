#include "game.h"
#include <regex>
#include <iostream>
#include <cassert>

game::game(std::string input)
{
    const static std::regex comment_pattern(R"(\{.*?\})");
    const static std::regex metadata_pattern(R"(\[([^:]*)\s([^:]*)\])");
    const static std::regex block_pattern(R"(\[[^\[\]]*\])");
    std::string clean_input = std::regex_replace(input, comment_pattern, "");
    std::smatch block_match;
    while(std::regex_search(clean_input, block_match, block_pattern))
    {
        std::smatch sm;
        std::string str = block_match.str();
        
        if(std::regex_search(str, sm, metadata_pattern))
        {
            metadata[sm[1]] = sm[2];
            //cerr << "Key: " << sm[1] << "\tValue: " << sm[2] << endl;
        }
        clean_input = block_match.suffix(); //all it to search the remaining parts
    }

    multiverse m(input);
    cached_states.push_back(state(m));
    now = cached_states.begin();
}

state game::get_current_state() const
{
    return *now;
}

void game::undo()
{
    if(now != cached_states.begin())
        now--;
}

void game::redo()
{
    if(now+1 != cached_states.end())
        now++;
}

bool game::apply_move(full_move fm)
{
    state new_state = *now;
            
    bool flag = new_state.apply_move(fm);
    
    // Remove all future states after `now`
    cached_states.erase(now + 1, cached_states.end());
    
    // Add new state to history
    cached_states.push_back(new_state);
    now = cached_states.end() - 1;
    return flag;
}
