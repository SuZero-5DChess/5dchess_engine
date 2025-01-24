#include "multiverse.h"
#include <regex>
#include <sstream>
#include <algorithm>
#include <limits>

#include <iostream>

using std::cerr;
using std::endl;

// use the bijection from integers to non-negative integers:
// x -> ~(x>>1)
constexpr int l_to_u(int l)
{
    if(l >= 0)
        return l << 1;
    else
        return ~(l << 1);
}

constexpr int tc_to_v(int t, int c)
{
    return t << 1 | c;
}

constexpr int u_to_l(int u)
{
    if(u & 1)
        return ~(u >> 1);
    else
        return u >> 1;
}

constexpr std::tuple<int, int> v_to_tc(int v)
{
    return std::tuple<int, int>(v >> 1, v & 1);
}


multiverse::multiverse(const std::string &input)
{
    const static std::regex comment_pattern(R"(\{.*?\})");
    const static std::regex block_pattern(R"(\[[^\[\]]*\])");
    const static std::regex metadata_pattern(R"(\[([^:]*)\s([^:]*)\])");
    const static std::regex board_pattern(R"(\[(.+?):([+-]?\d+):([+-]?\d+):([a-zA-Z])\])");

    std::string clean_input = std::regex_replace(input, comment_pattern, "");
    std::smatch block_match;
    l_min = l_max = 0;
    while(std::regex_search(clean_input, block_match, block_pattern))
    {
        std::smatch sm;
        std::string str = block_match.str();
        if(std::regex_search(str, sm, metadata_pattern))
        {
            this->metadata[sm[1]] = sm[2];
            //cerr << "Key: " << sm[1] << "\tValue: " << sm[2] << endl;
        }
        else if(std::regex_search(str, sm, board_pattern))
        {
            int l = std::stoi(sm[2]);
            int t = std::stoi(sm[3]);
            int c = 0;
            switch(sm[4].str()[0])
            {
            case 'w':
            case 'W':
                c = 0;
                break;
            case 'b':
            case 'B':
                c = 1;
                break;
            default:
                throw std::runtime_error("Unknown color:" + sm[4].str() + " in " + str);
                break;
            }
            int u = l_to_u(l);
            int v = tc_to_v(t, c);

            // if u is too large, resize this->board to accommodate new board
            // and fill any missing row with empty vector
            if(u >= this->boards.size()) 
            {
                this->boards.resize(u+1, vector<shared_ptr<board>>());
                this->timeline_start.resize(u+1, std::numeric_limits<int>::max());
                this->timeline_end.resize(u+1, std::numeric_limits<int>::min());
            }
            l_min = std::min(l_min, l);
            l_max = std::max(l_max, l);
            vector<shared_ptr<board>> &timeline = this->boards[u];
            // do the same for v
            if(v >= timeline.size())
            {
                timeline.resize(v+1, nullptr);
            }
            else if(v < 0)
            {
                throw std::runtime_error("Negative time is not supported. " + str);
            }
            timeline[v] = std::make_shared<board>(sm[1]);
            timeline_start[u] = std::min(timeline_start[u], v);
            timeline_end[u]   = std::max(timeline_end[u],   v);
        }
        clean_input = block_match.suffix(); //all it to search the remaining parts
    }
    int tmp = std::min(-l_min, l_max);
    number_activated = tmp < std::max(-l_min, l_max) ? tmp+1 : tmp;
    check_multiverse_shape();
}

bool multiverse::check_multiverse_shape()
{
    for(int l = l_min; l <= l_max; l++)
    {
        int u = l_to_u(l);
        if(boards[u].empty())
            return false;
        for(int v = timeline_start[u]; v <= timeline_end[u]; v++)
        {
            if(boards[u][v] == nullptr)
                return false;
        }
    }
    present = std::numeric_limits<int>::max();
    for(int l = std::max(l_min, -number_activated); l <= std::min(l_max, number_activated); l++)
    {
        present = std::min(present, timeline_end[l_to_u(l)]);
    }
    return true;
}

/*
board5d::~board5d()
{
    cerr << "5d Board destroyed" << endl;
    for(const auto &colored_line : boards)
    {
        for(auto ptr : colored_line)
        {
            ptr.reset();
        }
    }
}
 */

shared_ptr<board> multiverse::get_board(int l, int t, int c) const
{
    try
    {
        return this->boards.at(l_to_u(l)).at(tc_to_v(t,c));
    }
    catch(const std::out_of_range& ex)
    {
        return nullptr;
    }
}

vector<tuple<int,int,int,string>> multiverse::get_boards() const
{
    vector<tuple<int,int,int,string>> result;
    for(int u = 0; u < this->boards.size(); u++)
    {
        const auto& timeline = this->boards[u];
        int l = u_to_l(u);
        for(int v = 0; v < timeline.size(); v++)
        {
            const auto [t, c] = v_to_tc(v);
            if(timeline[v] != nullptr)
            {
                result.push_back(tuple<int,int,int,string>(l,t,c,timeline[v]->get_fen()));
            }
        }
    }
    return result;
}

string multiverse::to_string() const
{
    std::stringstream sstm;
    const auto& [t,c] = v_to_tc(present);
    sstm << "Present: T" << t << (c?'b':'w') << "\n";
    for(int u = 0; u < this->boards.size(); u++)
    {
        const auto& timeline = this->boards[u];
        int l = u_to_l(u);
        for(int v = 0; v < timeline.size(); v++)
        {
            const auto [t, c] = v_to_tc(v);
            if(timeline[v] != nullptr)
            {
                sstm << "L" << l << "T" << t << (c ? 'b' : 'w') << "\n";
                sstm << timeline[v]->to_string();
            }
        }
    }
    return sstm.str();
}

bool multiverse::inbound(vec4 a, int color) const
{
    int l = a.l(), u = l_to_u(l), v = tc_to_v(a.t(), color);
    if(a.outbound() || l < l_min || l > l_max)
        return false;
    return timeline_start[u] <= v && v <= timeline_end[u];
}

piece_t multiverse::get_piece(vec4 a, int color) const
{
    return (*boards[l_to_u(a.l())][tc_to_v(a.t(), color)])[a.xy()];
}

const vector<vec4> knight_delta = {vec4( 2, 1, 0, 0), vec4( 2, 0, 1, 0), vec4( 2, 0, 0, 1), vec4( 1, 2, 0, 0), vec4( 1, 0, 2, 0), vec4( 1, 0, 0, 2), vec4( 0, 2, 1, 0), vec4( 0, 2, 0, 1), vec4( 0, 1, 2, 0), vec4( 0, 1, 0, 2), vec4( 0, 0, 2, 1), vec4( 0, 0, 1, 2), vec4(-2, 1, 0, 0), vec4(-2, 0, 1, 0), vec4(-2, 0, 0, 1), vec4( 1,-2, 0, 0), vec4( 1, 0,-2, 0), vec4( 1, 0, 0,-2), vec4( 0,-2, 1, 0), vec4( 0,-2, 0, 1), vec4( 0, 1,-2, 0), vec4( 0, 1, 0,-2), vec4( 0, 0,-2, 1), vec4( 0, 0, 1,-2), vec4( 2,-1, 0, 0), vec4( 2, 0,-1, 0), vec4( 2, 0, 0,-1), vec4(-1, 2, 0, 0), vec4(-1, 0, 2, 0), vec4(-1, 0, 0, 2), vec4( 0, 2,-1, 0), vec4( 0, 2, 0,-1), vec4( 0,-1, 2, 0), vec4( 0,-1, 0, 2), vec4( 0, 0, 2,-1), vec4( 0, 0,-1, 2), vec4(-2,-1, 0, 0), vec4(-2, 0,-1, 0), vec4(-2, 0, 0,-1), vec4(-1,-2, 0, 0), vec4(-1, 0,-2, 0), vec4(-1, 0, 0,-2), vec4( 0,-2,-1, 0), vec4( 0,-2, 0,-1), vec4( 0,-1,-2, 0), vec4( 0,-1, 0,-2), vec4( 0, 0,-2,-1), vec4( 0, 0,-1,-2)};

namespace views = std::ranges::views;

vector<vec4> multiverse::gen_piece_move(const vec4& p, int board_color) const
{
    const piece_t pic = get_piece(p, board_color);
    const bool p_color = get_color(pic);
    const piece_t p_piece = to_white(pic);
    vector<vec4> result;
    auto can_go_to = [&](vec4 d)
    {
        piece_t q_piece = get_piece(p+d, board_color);
        return q_piece == NO_PIECE || p_color != get_color(q_piece);
    };
    auto delta_in_range = [&](vec4 d)
    {
        return inbound(p+d, board_color);
    };
    std::cerr << "---" << p_piece << std::endl;
    switch(p_piece)
    {
        case KNIGHT_B:
            result = knight_delta
            | views::filter(delta_in_range)
            | views::filter(can_go_to)
            | ranges::to<vector>();
            break;
        default:
            std::cerr << "gen_piece_move:" << p_piece << "not implemented" << std::endl;
    }
    return result;
}

