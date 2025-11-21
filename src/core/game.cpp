#include "game.h"
#include <regex>
#include <iostream>
#include <ranges>
#include <algorithm>
#include <cassert>
#include <variant>

std::vector<move5d> pgn_to_moves(const std::string& input)
{
    // Regex to match slashes ("/") and move numbers (e.g., "1.", "2.")
    const static std::regex pattern(R"((\d+\.)|(/))");
    std::string output = std::regex_replace(input, pattern, " submit ");
    // Split the result by whitespace into a vector of moves
    std::vector<move5d> result;
    std::istringstream result_stream(output);
    std::string word;
    //remove the first "submit"
    result_stream >> word;
    while (result_stream >> word)
    {
        if(word.compare("submit") != 0)
        {
            result.push_back(move5d(word));
        }
        else
        {
            result.push_back(move5d::submit());
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
    
    multiverse_odd m(input, size_x, size_y);
    cached_states.push_back(state(m));
    now = cached_states.begin();
    
    // apply the moves stated in input string
    std::vector<move5d> moves = pgn_to_moves(clean_input);
    for(const auto &move : moves)
    {
//        std::cerr << "trying to apply move " << move << "\n";
//        std::cerr << "before: " << now->get_present().first << ", ";
        bool flag = apply_move(move);
//        std::cerr << "present: " << now->get_present() << "\n";
//        std::cerr << now->to_string();
//        std::cerr << "apparent present: " << now->apparent_present() << "\n";
        if(!flag)
        {
            state cs = get_current_state();
            std::cerr << "game(): In state " << cs.to_string() << "\n";
            std::cerr << "trying to apply move " << move << "\n";
            print_range("The movable pieces are", cs.gen_movable_pieces());
            if (std::holds_alternative<full_move>(move.data))
            {
                auto fm = std::get<full_move>(move.data);
                print_range("the allowed moves are: \n", gen_move_if_playable(fm.from));
            }
            throw std::runtime_error("failed to apply move: " + move.to_string());
        }
    }
}

std::tuple<int,int> game::get_current_present() const
{
    return get_current_state().get_present();
}

state game::get_current_state() const
{
    return *now;
}

std::vector<std::tuple<int, int, bool, std::string>> game::get_current_boards() const
{
    return get_current_state().get_boards();
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
        std::vector<vec4> result;
        for(vec4 v : cs.gen_piece_move(p))
        {
            result.push_back(v);
        }
        return result;
    }
    else
    {
        return std::vector<vec4>();
    }
}

match_status_t game::get_match_status() const
{
    return match_status_t();
}

std::vector<vec4> game::get_movable_pieces() const
{
    state s = get_current_state();
    return s.gen_movable_pieces();
}

bool game::is_playable(vec4 p) const
{
    auto [mandatory_timelines, optional_timelines, unplayable_timelines] = get_current_timeline_status();
    const state& cs = get_current_state();
    if(std::ranges::contains(mandatory_timelines, p.l())
    || std::ranges::contains(optional_timelines, p.l()))
    {
        auto [t, c] = cs.get_present();
        auto v1 = std::make_pair(p.t(), c);
        auto v2 = cs.get_timeline_end(p.l());
        if(v1 == v2)
        {
            piece_t p_piece = cs.get_piece(p, c);
            if(p_piece != NO_PIECE && p_piece != WALL_PIECE)
            {
                return c == static_cast<int>(piece_color(p_piece));
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
    return get_current_state().can_submit().has_value();
}

bool game::undo()
{
    bool flag = can_undo();
    if(flag)
        now--;
    return flag;
}

bool game::redo()
{
    bool flag = can_undo();
    if(flag)
        now--;
    return flag;
}

bool game::apply_move(move5d mv)
{
    std::optional<state> ans;
    if (std::holds_alternative<full_move>(mv.data))
    {
        auto fm = std::get<full_move>(mv.data);
        vec4 p = fm.from;
        if(!is_playable(p))
        {
            return false;
        }
        ans = now->can_apply(fm);
    }
    else if (std::holds_alternative<std::monostate>(mv.data))
    {
        ans = now->can_submit();
    }
    
    bool flag = ans.has_value();
    if(ans)
    {
        state new_state = std::move(ans.value());
        cached_states.erase(now + 1, cached_states.end());
        cached_states.push_back(new_state);
        now = cached_states.end() - 1;
    }
    return flag;
}

// bool game::apply_indicator_move(move5d fm)
// {
//     state new_state = *now;
//     if (std::holds_alternative<std::tuple<vec4,vec4>>(fm.data))
//     {
//         auto [p,d] = std::get<std::tuple<vec4,vec4>>(fm.data);
//         if(!is_playable(p))
//         {
//             return false;
//         }
//     }
//     new_state.apply_move(fm);
    
//     cached_states.erase(now + 1, cached_states.end());
//     cached_states.push_back(new_state);
//     now = cached_states.end() - 1;
//     return true;
// }

void game::set_promotion_piece(piece_t pt)
{
    now->set_promotion_piece(pt);
}

bool game::currently_check() const
{
    return get_current_state().find_checks().first().has_value();
}

std::vector<std::pair<vec4, vec4>> game::get_current_checks() const
{
    std::vector<std::pair<vec4, vec4>> result;
    for(full_move fm : get_current_state().find_checks())
    {
        result.push_back(std::make_pair(fm.from, fm.to));
    }
    return result;
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
