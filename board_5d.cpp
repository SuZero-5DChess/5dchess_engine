#include "board_5d.h"
#include <regex>
#include <sstream>

// use the bijection from integers to non-negative integers:
// x -> ~(x>>1)
constexpr int lpos(int l, int color)
{
    if(l>=0)
        return l<<2 | color;
    else
        return (~(l<<1))<<1 | color;
}

board5d::board5d(const std::string &input)
{
    const static std::regex comment_pattern(R"(\{.*?\})");
    const static std::regex block_pattern(R"(\[[^\[\]]*\])");
    const static std::regex metadata_pattern(R"(\[([^:]*):([^:]*)\])");
    const static std::regex board_pattern(R"(\[(.+?):([+-]?\d+):([+-]?\d+):([a-zA-Z])\])");

    std::string clean_input = std::regex_replace(input, comment_pattern, "");
    std::smatch block_match;
    while(std::regex_search(clean_input, block_match, block_pattern))
    {
        std::smatch sm;
        std::string s = block_match.str();
        if(std::regex_search(s, sm, metadata_pattern))
        {
            this->metadata[sm[1]] = sm[2];
            //cout << "Key: " << sm[1] << "\tValue: " << sm[2] << endl;
        }
        else if(std::regex_search(s, sm, board_pattern))
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
                throw std::runtime_error("Unknown color:" + sm[4].str() + " in " + s);
                break;
            }
            int s = lpos(l, c);

            //if s is too large, resize this->board to accommodate new board
            //and fill any missing row with empty vector
            if(s >= this->boards.size()) 
            {
                this->boards.resize(s+1, vector<shared_ptr<board2d>>());
            }
            vector<shared_ptr<board2d>> &colored_line = this->boards[s];
            if(t >= colored_line.size())
            {
                colored_line.resize(t+1, nullptr);
            }
            else if(t < 0)
            {
                throw std::runtime_error("Negative timeline is not supported. " + s);
            }
            colored_line[t] = std::make_shared<board2d>(sm[1]);
            //cout << "FEN: " << sm[1] << "\tL" << sm[2] << "T" << sm[3] << sm[4] << endl;
        }
        clean_input = block_match.suffix();
    }
}



shared_ptr<board2d> board5d::get_board(int t, int l, int c) const
{
    return this->boards[lpos(l,c)][t];
}

string board5d::to_string() const
{
    std::stringstream sstm;
    for(int s = 0; s < this->boards.size(); s++)
    {
        const auto& colored_timeline = this->boards[s];
        int c = s & 0x1;
        int l = s & 0x2 ? (~(s>>1))>>1 : s >> 2; 
        for(int t = 0; t < colored_timeline.size(); t++)
        {
            if(colored_timeline[t] != nullptr)
            {
                sstm << "L" << l << "T" << t << (c ? 'b' : 'w') << "\n";
                sstm << colored_timeline[t]->to_string();
            }
        }
    }
    return string();
}
