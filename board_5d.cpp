#include "board_5d.h"
#include <regex>
#include <sstream>

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


board5d::board5d(const std::string &input)
{
    const static std::regex comment_pattern(R"(\{.*?\})");
    const static std::regex block_pattern(R"(\[[^\[\]]*\])");
    const static std::regex metadata_pattern(R"(\[([^:]*)\s([^:]*)\])");
    const static std::regex board_pattern(R"(\[(.+?):([+-]?\d+):([+-]?\d+):([a-zA-Z])\])");

    std::string clean_input = std::regex_replace(input, comment_pattern, "");
    std::smatch block_match;
    while(std::regex_search(clean_input, block_match, block_pattern))
    {
        std::smatch sm;
        std::string str = block_match.str();
        if(std::regex_search(str, sm, metadata_pattern))
        {
            this->metadata[sm[1]] = sm[2];
            cerr << "Key: " << sm[1] << "\tValue: " << sm[2] << endl;
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
                this->boards.resize(u+1, vector<shared_ptr<board2d>>());
            }
            vector<shared_ptr<board2d>> &timeline = this->boards[u];
            // do the same for v
            if(v >= timeline.size())
            {
                timeline.resize(v+1, nullptr);
            }
            else if(v < 0)
            {
                throw std::runtime_error("Negative timeline is not supported. " + str);
            }
            timeline[v] = std::make_shared<board2d>(sm[1]);
            cerr << "FEN: " << sm[1] << "\tL" << sm[2] << "T" << sm[3] << sm[4] << endl;
            cerr << "written to \tu = " << u << "\tv = " << v << endl;
            cerr << boards[u][v]->to_string();
        }
        clean_input = block_match.suffix(); //all it to search the remaining parts
    }
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

shared_ptr<board2d> board5d::get_board(int t, int l, int c) const
{
    return this->boards[l_to_u(l)][tc_to_v(t,c)];
}

string board5d::to_string() const
{
    std::stringstream sstm;
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
