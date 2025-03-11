#include "multiverse.h"
#include "utils.h"
#include <regex>
#include <sstream>
#include <algorithm>
#include <limits>
#include <iostream>
#include <utility>


using std::tuple;
using std::vector;
using std::string;
using std::shared_ptr;
namespace ranges = std::ranges;
using std::cerr;
using std::endl;


multiverse::multiverse(const std::string &input)
{
    const static std::regex comment_pattern(R"(\{.*?\})");
    const static std::regex block_pattern(R"(\[[^\[\]]*\])");
    const static std::regex board_pattern(R"(\[(.+?):([+-]?\d+):([+-]?\d+):([a-zA-Z])\])");

    std::string clean_input = std::regex_replace(input, comment_pattern, "");
    std::smatch block_match;
    l_min = l_max = 0;
    while(std::regex_search(clean_input, block_match, block_pattern))
    {
        std::smatch sm;
        std::string str = block_match.str();
        
        if(std::regex_search(str, sm, board_pattern))
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
            insert_board(l, t, c, std::make_shared<board>(sm[1]));
        }
        clean_input = block_match.suffix(); //all it to search the remaining parts
    }
    for(int l = l_min; l <= l_max; l++)
    {
        int u = l_to_u(l);
        if(boards[u].empty())
            throw std::runtime_error("Error: There is a gap between timelines.");
        for(int v = timeline_start[u]; v <= timeline_end[u]; v++)
        {
            if(boards[u][v] == nullptr)
            {
                throw std::runtime_error("Error: There is a gap between boards on timeline L"
                    + std::to_string(u_to_l(u)) + ".");
            }
        }
    }
}

std::tuple<int,int,int> multiverse::get_present() const
{
    int tmp = std::min(-l_min, l_max);
    int number_activated = tmp < std::max(-l_min, l_max) ? tmp+1 : tmp;
    int present_v = std::numeric_limits<int>::max();
    for(int l = std::max(l_min, -number_activated); l <= std::min(l_max, number_activated); l++)
    {
        present_v = std::min(present_v, timeline_end[l_to_u(l)]);
    }
    return std::tuple_cat(std::tie(number_activated), v_to_tc(present_v));
}

bool multiverse::is_active(int l) const
{
    int number_activated = std::get<0>(get_present());
    return std::max(l_min, -number_activated) <= l
        && std::min(l_max, number_activated) >= l;
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

void multiverse::append_board(int l, const shared_ptr<board>& b_ptr)
{
    int u = l_to_u(l);
    boards[u].push_back(b_ptr);
    timeline_end[u]++;
}

void multiverse::insert_board(int l, int t, int c, const shared_ptr<board>& b_ptr)
{
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
        throw std::runtime_error("Negative time is not supported.");
    }
    timeline[v] = b_ptr;
    timeline_start[u] = std::min(timeline_start[u], v);
    timeline_end[u]   = std::max(timeline_end[u],   v);
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
    auto [_, present, player] = get_present();
    sstm << "Present: T" << present << (player?'b':'w') << "\n";
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
const vector<vec4> rook_delta = {vec4( 1, 0, 0, 0), vec4(-1, 0, 0, 0), vec4( 0, 1, 0, 0), vec4( 0,-1, 0, 0), vec4( 0, 0, 1, 0), vec4( 0, 0,-1, 0), vec4( 0, 0, 0, 1), vec4( 0, 0, 0,-1)};
const vector<vec4> bishop_delta = {vec4( 1, 1, 0, 0), vec4(-1, 1, 0, 0), vec4( 1,-1, 0, 0), vec4(-1,-1, 0, 0), vec4( 1, 0, 1, 0), vec4(-1, 0, 1, 0), vec4( 1, 0,-1, 0), vec4(-1, 0,-1, 0), vec4( 1, 0, 0, 1), vec4(-1, 0, 0, 1), vec4( 1, 0, 0,-1), vec4(-1, 0, 0,-1), vec4( 0, 1, 1, 0), vec4( 0,-1, 1, 0), vec4( 0, 1,-1, 0), vec4( 0,-1,-1, 0), vec4( 0, 1, 0, 1), vec4( 0,-1, 0, 1), vec4( 0, 1, 0,-1), vec4( 0,-1, 0,-1), vec4( 0, 0, 1, 1), vec4( 0, 0,-1, 1), vec4( 0, 0, 1,-1), vec4( 0, 0,-1,-1)};
const vector<vec4> unicorn_delta = {vec4( 1, 1, 1, 0), vec4(-1, 1, 1, 0), vec4( 1,-1, 1, 0), vec4(-1,-1, 1, 0), vec4( 1, 1,-1, 0), vec4(-1, 1,-1, 0), vec4( 1,-1,-1, 0), vec4(-1,-1,-1, 0), vec4( 1, 1, 0, 1), vec4(-1, 1, 0, 1), vec4( 1,-1, 0, 1), vec4(-1,-1, 0, 1), vec4( 1, 1, 0,-1), vec4(-1, 1, 0,-1), vec4( 1,-1, 0,-1), vec4(-1,-1, 0,-1), vec4( 1, 0, 1, 1), vec4(-1, 0, 1, 1), vec4( 1, 0,-1, 1), vec4(-1, 0,-1, 1), vec4( 1, 0, 1,-1), vec4(-1, 0, 1,-1), vec4( 1, 0,-1,-1), vec4(-1, 0,-1,-1), vec4( 0, 1, 1, 1), vec4( 0,-1, 1, 1), vec4( 0, 1,-1, 1), vec4( 0,-1,-1, 1), vec4( 0, 1, 1,-1), vec4( 0,-1, 1,-1), vec4( 0, 1,-1,-1), vec4( 0,-1,-1,-1)};
const vector<vec4> dragon_delta = {vec4( 1, 1, 1, 1), vec4(-1, 1, 1, 1), vec4( 1,-1, 1, 1), vec4(-1,-1, 1, 1), vec4( 1, 1,-1, 1), vec4(-1, 1,-1, 1), vec4( 1,-1,-1, 1), vec4(-1,-1,-1, 1), vec4( 1, 1, 1,-1), vec4(-1, 1, 1,-1), vec4( 1,-1, 1,-1), vec4(-1,-1, 1,-1), vec4( 1, 1,-1,-1), vec4(-1, 1,-1,-1), vec4( 1,-1,-1,-1), vec4(-1,-1,-1,-1)};
const vector<vec4> queen_delta = concat_vectors(rook_delta, bishop_delta, unicorn_delta, dragon_delta);
const vector<vec4> pawn_unmoved_white_delta = {vec4( 0, 2, 0, 0), vec4( 0, 0, 0,-2)};
const vector<vec4> pawn_white_delta = {vec4( 0, 1, 0, 0), vec4( 0, 0, 0,-1)};
const vector<vec4> pawn_white_capture_delta = {vec4( 1, 1, 0, 0), vec4(-1, 1, 0, 0), vec4( 0, 0, 1,-1), vec4( 0, 0,-1,-1)};
const vector<vec4> pawn_unmoved_black_delta = {vec4( 0,-2, 0, 0), vec4( 0, 0, 0, 2)};
const vector<vec4> pawn_black_delta = {vec4( 0,-1, 0, 0), vec4( 0, 0, 0, 1)};
const vector<vec4> pawn_black_capture_delta = {vec4( 1,-1, 0, 0), vec4(-1,-1, 0, 0), vec4( 0, 0, 1, 1), vec4( 0, 0,-1, 1)};

namespace views = std::ranges::views;

vector<vec4> multiverse::gen_piece_move(const vec4& p, int board_color) const
{
    const piece_t p_piece = get_piece(p, board_color);
    const bool p_color = get_color(p_piece);
    vector<vec4> result;
    auto is_blank = [&](vec4 d)
    {
        piece_t q_piece = get_piece(p+d, board_color);
        return q_piece == NO_PIECE;
    };
    auto can_go_to = [&](vec4 d)
    {
        piece_t q_piece = get_piece(p+d, board_color);
        return q_piece == NO_PIECE || p_color != get_color(q_piece);
    };
    auto delta_in_range = [&](vec4 d)
    {
        return inbound(p+d, board_color);
    };
    auto good_castling_direction = [&](vec4 d)
    {
        for(vec4 c = d; delta_in_range(d); c = c + d)
        {
            piece_t q_piece = get_piece(p+d, board_color);
            if(q_piece == ROOK_UW || q_piece == ROOK_UB)
            {
                return true;
            }
            else if(q_piece != NO_PIECE)
            {
                return false;
            }
        }
        return false;
    };
    auto multiple_moves = [&](const vector<vec4>& delta)
    {
        vector<vec4> res;
        //I wold prefer to write `vector<tuple<vec4,vec4>>` instead of `auto`, but clang++ and visual studio complain about different type errors
        auto zipped_delta = std::views::zip(vector<vec4>(delta.size(), vec4(0,0,0,0)), delta) | ranges::to<vector>();
        while(!zipped_delta.empty())
        {
            for(std::tuple<vec4&, vec4> m : zipped_delta)
            {
                std::get<0>(m) = std::get<0>(m) + std::get<1>(m);
            }
            zipped_delta = zipped_delta | views::filter([&](auto x){
                vec4 d = std::get<0>(x);
                return delta_in_range(d);
            }) | ranges::to<vector>();
            //print_range("after checking range:", zipped_delta);
            auto tmp = zipped_delta
            | views::transform([](auto elem){return std::get<0>(elem);})
            | views::filter(can_go_to)
            | ranges::to<vector>();
            //print_range("tmp:", tmp);
            append_vectors(res, tmp);
            zipped_delta = zipped_delta | views::filter([&](auto x){
                vec4 d = std::get<0>(x);
                return is_blank(d);
            }) | ranges::to<vector>();
        }
        return res;
    };
    //std::cerr << "---" << piece_name(p_piece) << std::endl;
    switch(p_piece)
    {
        case KNIGHT_W:
        case KNIGHT_B:
            result = knight_delta
            | views::filter(delta_in_range)
            | views::filter(can_go_to)
            | ranges::to<vector>();
            break;
        case ROOK_UW:
        case ROOK_UB:
        case ROOK_W:
        case ROOK_B:
            result = multiple_moves(rook_delta);
            break;
        case BISHOP_W:
        case BISHOP_B:
            result = multiple_moves(bishop_delta);
            break;
        case UNICORN_W:
        case UNICORN_B:
            result = multiple_moves(unicorn_delta);
            break;
        case DRAGON_W:
        case DRAGON_B:
            result = multiple_moves(dragon_delta);
            break;
        case QUEEN_W:
        case QUEEN_B:
            result = multiple_moves(queen_delta);
            break;
        case KING_UW:
        case KING_UB:{
            //TODO: castling passage check
            result = vector<vec4>{vec4(-1,0,0,0), vec4(1,0,0,0)}
            | views::filter(good_castling_direction)
            | views::transform([](vec4 v){return v+v;})
            | ranges::to<vector>();
        }
        case KING_W:
        case KING_B:
            append_vectors(result, queen_delta
            | views::filter(delta_in_range)
            | views::filter(can_go_to)
            | ranges::to<vector>());
            break;
        case PAWN_UW:
            for(int i = 0; i < 2; i++)
            {
                if(!inbound(p+pawn_unmoved_white_delta[i], board_color))
                {
                    continue;
                }
                piece_t q_piece = get_piece(p+pawn_white_delta[i], board_color);
                piece_t r_piece = get_piece(p+pawn_unmoved_white_delta[i], board_color);
                if(q_piece == NO_PIECE && r_piece==NO_PIECE)
                {
                    result.push_back(pawn_unmoved_white_delta[i]);
                }
            }
        case PAWN_W:
            for(const vec4& d : pawn_white_delta)
            {
                if(inbound(p+d, board_color) && get_piece(p+d, board_color) == NO_PIECE)
                {
                    result.push_back(d);
                }
            }
            for(const vec4& d : pawn_white_capture_delta)
            {
                if(!inbound(p+d, board_color))
                {
                    continue;
                }
                piece_t q_name = get_piece(p+d, board_color);
                // normal capture
                if(q_name != NO_PIECE && p_color!=get_color(q_name))
                {
                    result.push_back(d);
                }
                // en passant
                else if(q_name == NO_PIECE && get_piece(p+vec4(d.x(),0,0,0), board_color) == PAWN_B
                    && inbound(p+vec4(d.x(),2,-1,0), board_color)
                    && get_piece(p+vec4(d.x(),0,-1,0), board_color) == NO_PIECE
                    && get_piece(p+vec4(d.x(),2,-1,0), board_color) == PAWN_UB)
                {
                    result.push_back(d);
                }
            }
            break;
        case PAWN_UB:
            for(int i = 0; i < 2; i++)
            {
                if(!inbound(p+pawn_unmoved_black_delta[i], board_color))
                {
                    continue;
                }
                piece_t q_piece = get_piece(p+pawn_black_delta[i], board_color);
                piece_t r_piece = get_piece(p+pawn_unmoved_black_delta[i], board_color);
                if(q_piece == NO_PIECE && r_piece==NO_PIECE)
                {
                    result.push_back(pawn_unmoved_black_delta[i]);
                }
            }
        case PAWN_B:
            for(const vec4& d : pawn_black_delta)
            {
                if(inbound(p+d, board_color) && get_piece(p+d, board_color) == NO_PIECE)
                {
                    result.push_back(d);
                }
            }
            for(const vec4& d : pawn_black_capture_delta)
            {
                if(!inbound(p+d, board_color))
                {
                    continue;
                }
                piece_t q_name = get_piece(p+d, board_color);
                // normal capture
                if(q_name != NO_PIECE && p_color!=get_color(q_name))
                {
                    result.push_back(d);
                }
                // en passant
                else if(q_name == NO_PIECE && get_piece(p+vec4(d.x(),0,0,0), board_color) == PAWN_W
                    && inbound(p+vec4(d.x(),2,-1,0), board_color)
                    && get_piece(p+vec4(d.x(),0,-1,0), board_color) == NO_PIECE
                    && get_piece(p+vec4(d.x(),2,-1,0), board_color) == PAWN_UW)
                {
                    result.push_back(d);
                }
            }
            break;
        default:
            std::cerr << "gen_piece_move:" << p_piece << "not implemented" << std::endl;
    }
    return result;
}

