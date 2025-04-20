#include "game.h"
#include <regex>
#include <iostream>
#include <ranges>
#include <algorithm>
#include <cassert>
#include <variant>

std::vector<full_move> pgn_to_moves(const std::string& input)
{
    // Regex to match slashes ("/") and move numbers (e.g., "1.", "2.")
    const static std::regex pattern(R"((\d+\.)|(/))");
    std::string output = std::regex_replace(input, pattern, " submit ");
    // Split the result by whitespace into a vector of moves
    std::vector<full_move> result;
    std::istringstream result_stream(output);
    std::string word;
    //remove the first "submit"
    result_stream >> word;
    while (result_stream >> word)
    {
        if(word.compare("submit") != 0)
        {
            result.push_back(full_move(word));
        }
        else
        {
            result.push_back(full_move::submit());
        }
    }
    return result;
}

game::game(std::string input)
{
    const static std::regex comment_pattern(R"(\{.*?\})");
    const static std::regex metadata_pattern(R"%(\[([^:]*)\s"([^:]*)"\])%");
    const static std::regex block_pattern(R"(\[[^\[\]]*\])");
    std::string clean_input = std::regex_replace(input, comment_pattern, "");
    std::smatch block_match;
    while(std::regex_search(clean_input, block_match, block_pattern))
    {
        std::smatch sm;
        std::string str = block_match.str();
        
        if(std::regex_search(str, sm, metadata_pattern))
        {
            std::string s = sm[1];
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){
                return std::tolower(c);
            }); // keys are always stored in lower cases
            metadata[s] = sm[2];
            //cerr << "Key: " << sm[1] << "\tValue: " << sm[2] << endl;
        }
        clean_input = block_match.suffix(); //all it to search the remaining parts
    }
    
    metadata.try_emplace("size", "8x8");
    auto [size_x, size_y] = get_board_size();
    
    multiverse m(input, size_x, size_y);
    cached_states.push_back(state(m));
    now = cached_states.begin();
    
    // apply the moves stated in input string
    std::vector<full_move> moves = pgn_to_moves(clean_input);
    for(const auto &move : moves)
    {
        apply_move(move);
    }
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
    if(is_playable(p))
    {
        const state& cs = get_current_state();
        return cs.m.gen_piece_move(p, cs.player);
    }
    else
    {
        return std::vector<vec4>();
    }
}

match_status_t game::get_match_status() const
{
    return get_current_state().match_status;
}

std::vector<vec4> game::get_movable_pieces() const
{
    state s = get_current_state();
    std::vector<vec4> v;
    for(const auto &[p0, bb] : s.gen_movable_pieces())
    {
        for(int pos : marked_pos(bb))
        {
            v.push_back(vec4(pos, p0));
        }
    }
    return v;
}

bool game::is_playable (vec4 p) const
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
            piece_t p_piece = cs.m.get_piece(p, cs.player);
            if(p_piece != NO_PIECE && p_piece != WALL_PIECE)
            {
                return cs.player == static_cast<int>(piece_color(p_piece));
            }
        }
    }
    return false;
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
    if (std::holds_alternative<std::tuple<vec4,vec4>>(fm.data))
    {
        auto [p,d] = std::get<std::tuple<vec4,vec4>>(fm.data);
        if(!is_playable(p))
        {
            return false;
        }
    }
    bool flag = new_state.apply_move(fm);
    
    if(flag)
    {
        cached_states.erase(now + 1, cached_states.end());
        cached_states.push_back(new_state);
        now = cached_states.end() - 1;
    }
    return flag;
}

bool game::apply_indicator_move(full_move fm)
{
    state new_state = *now;
    if (std::holds_alternative<std::tuple<vec4,vec4>>(fm.data))
    {
        auto [p,d] = std::get<std::tuple<vec4,vec4>>(fm.data);
        if(!is_playable(p))
        {
            return false;
        }
    }
    new_state.apply_move(fm);
    
    cached_states.erase(now + 1, cached_states.end());
    cached_states.push_back(new_state);
    now = cached_states.end() - 1;
    return true;
}

std::vector<std::pair<vec4, vec4>> game::get_current_checks() const
{
    return get_current_state().find_all_checks();
}

std::tuple<int, int> game::get_board_size() const
{
    int size_x, size_y;
    const static std::regex size_regex(R"(^\s*(\d+)\s*x\s*(\d+)\s*$)");
    std::smatch size_sm;
    if(std::regex_search(metadata.at("size"), size_sm, size_regex))
    {
        size_x = std::stoi(size_sm[1]);
        size_y = std::stoi(size_sm[2]);
    }
    else
    {
        throw std::runtime_error("game::get_board_size(): "
                                 "Invalid size in metadata: " + metadata.at("size"));
    }
    return std::make_tuple(size_x, size_y);
}
