#include "game.h"
#include <regex>
#include <iostream>
#include <ranges>
#include <algorithm>
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

std::tuple<int,int> game::get_current_present() const
{
    return std::make_tuple(get_current_state().present, get_current_state().player);
}

state game::get_current_state() const
{
    return *now;
}

std::vector<std::tuple<int, int, int, std::string>> game::get_current_boards() const
{
    return get_current_state().m.get_boards();
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> game::get_current_timeline_status() const
{
    return get_current_state().get_timeline_status();
}

std::vector<vec4> game::gen_move_if_playable(vec4 p)
{
    auto [mandatory_timelines, optional_timelines, unplayable_timelines] = get_current_timeline_status();
    const state& cs = get_current_state();
    if(std::ranges::contains(mandatory_timelines, p.l())
    || std::ranges::contains(optional_timelines, p.l()))
    {
        int v1 = multiverse::tc_to_v(p.t(), cs.player);
        int v2 = cs.m.timeline_end[multiverse::l_to_u(p.l())];
        if(v1 == v2)
        {
            return cs.m.gen_piece_move(p, cs.player);
        }
    }
    return std::vector<vec4>();
}

bool game::can_undo() const
{
    return now != cached_states.begin();
}

bool game::can_redo() const
{
    return now+1 != cached_states.end();
}

bool game::can_submit() const
{
    return get_current_state().can_submit();
}

void game::undo()
{
    if(can_undo())
        now--;
}

void game::redo()
{
    if(can_redo())
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
