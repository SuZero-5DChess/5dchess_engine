#include "multiverse.h"
#include "utils.h"
#include "magic.h"
#include <regex>
#include <sstream>
#include <algorithm>
#include <limits>
#include <iostream>
#include <utility>
#include <initializer_list>


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

int multiverse::number_activated() const
{
    int tmp = std::min(-l_min, l_max);
    if(tmp < std::max(-l_min, l_max))
    {
        return tmp + 1;
    }
    else
    {
        return tmp;
    }
}

std::tuple<int,int> multiverse::get_present() const
{
    int na = number_activated();
    int present_v = std::numeric_limits<int>::max();
    for(int l = std::max(l_min, -na); l <= std::min(l_max, na); l++)
    {
        present_v = std::min(present_v, timeline_end[l_to_u(l)]);
    }
    return v_to_tc(present_v);
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

std::shared_ptr<board> multiverse::get_board(int l, int t, int c) const
{
    try
    {
        return this->boards.at(l_to_u(l)).at(tc_to_v(t,c));
    }
    catch(const std::out_of_range& ex)
    {
		std::cerr << ex.what() << std::endl;
        std::cerr << ("Error: Out of range in multiverse::get_board( " + std::to_string(l) + ", " + std::to_string(t) + ")") << std::endl;
		throw std::runtime_error("Error: Out of range in multiverse::get_board( " + std::to_string(l) + ", " + std::to_string(t) + ")");
        return nullptr;
    }
}

void multiverse::append_board(int l, const std::shared_ptr<board>& b_ptr)
{
    int u = l_to_u(l);
    boards[u].push_back(b_ptr);
    timeline_end[u]++;
}

void multiverse::insert_board(int l, int t, int c, const std::shared_ptr<board>& b_ptr)
{
    int u = l_to_u(l);
    int v = tc_to_v(t, c);

    // if u is too large, resize this->board to accommodate new board
    // and fill any missing row with empty vector
    if(u >= this->boards.size())
    {
        this->boards.resize(u+1, std::vector<std::shared_ptr<board>>());
        this->timeline_start.resize(u+1, std::numeric_limits<int>::max());
        this->timeline_end.resize(u+1, std::numeric_limits<int>::min());
    }
    l_min = std::min(l_min, l);
    l_max = std::max(l_max, l);
    std::vector<std::shared_ptr<board>> &timeline = this->boards[u];
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

std::vector<std::tuple<int,int,int,std::string>> multiverse::get_boards() const
{
    std::vector<std::tuple<int,int,int,std::string>> result;
    for(int u = 0; u < this->boards.size(); u++)
    {
        const auto& timeline = this->boards[u];
        int l = u_to_l(u);
        for(int v = 0; v < timeline.size(); v++)
        {
            const auto [t, c] = v_to_tc(v);
            if(timeline[v] != nullptr)
            {
                result.push_back(std::make_tuple(l,t,c,timeline[v]->get_fen()));
            }
        }
    }
    return result;
}

std::string multiverse::to_string() const
{
    std::stringstream sstm;
    auto [present, player] = get_present();
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
    return boards[l_to_u(a.l())][tc_to_v(a.t(), color)]->get_piece(a.xy());
}

bool multiverse::get_umove_flag(vec4 a, int color) const
{
    return boards[l_to_u(a.l())][tc_to_v(a.t(), color)]->umove() & pmask(ppos(a.x(),a.y()));
}

#ifdef USE_LEAGACY_GENMOVE

const std::vector<vec4> knight_delta = {vec4( 2, 1, 0, 0), vec4( 2, 0, 1, 0), vec4( 2, 0, 0, 1), vec4( 1, 2, 0, 0), vec4( 1, 0, 2, 0), vec4( 1, 0, 0, 2), vec4( 0, 2, 1, 0), vec4( 0, 2, 0, 1), vec4( 0, 1, 2, 0), vec4( 0, 1, 0, 2), vec4( 0, 0, 2, 1), vec4( 0, 0, 1, 2), vec4(-2, 1, 0, 0), vec4(-2, 0, 1, 0), vec4(-2, 0, 0, 1), vec4( 1,-2, 0, 0), vec4( 1, 0,-2, 0), vec4( 1, 0, 0,-2), vec4( 0,-2, 1, 0), vec4( 0,-2, 0, 1), vec4( 0, 1,-2, 0), vec4( 0, 1, 0,-2), vec4( 0, 0,-2, 1), vec4( 0, 0, 1,-2), vec4( 2,-1, 0, 0), vec4( 2, 0,-1, 0), vec4( 2, 0, 0,-1), vec4(-1, 2, 0, 0), vec4(-1, 0, 2, 0), vec4(-1, 0, 0, 2), vec4( 0, 2,-1, 0), vec4( 0, 2, 0,-1), vec4( 0,-1, 2, 0), vec4( 0,-1, 0, 2), vec4( 0, 0, 2,-1), vec4( 0, 0,-1, 2), vec4(-2,-1, 0, 0), vec4(-2, 0,-1, 0), vec4(-2, 0, 0,-1), vec4(-1,-2, 0, 0), vec4(-1, 0,-2, 0), vec4(-1, 0, 0,-2), vec4( 0,-2,-1, 0), vec4( 0,-2, 0,-1), vec4( 0,-1,-2, 0), vec4( 0,-1, 0,-2), vec4( 0, 0,-2,-1), vec4( 0, 0,-1,-2)};
const std::vector<vec4> rook_delta = {vec4( 1, 0, 0, 0), vec4(-1, 0, 0, 0), vec4( 0, 1, 0, 0), vec4( 0,-1, 0, 0), vec4( 0, 0, 1, 0), vec4( 0, 0,-1, 0), vec4( 0, 0, 0, 1), vec4( 0, 0, 0,-1)};
const std::vector<vec4> bishop_delta = {vec4( 1, 1, 0, 0), vec4(-1, 1, 0, 0), vec4( 1,-1, 0, 0), vec4(-1,-1, 0, 0), vec4( 1, 0, 1, 0), vec4(-1, 0, 1, 0), vec4( 1, 0,-1, 0), vec4(-1, 0,-1, 0), vec4( 1, 0, 0, 1), vec4(-1, 0, 0, 1), vec4( 1, 0, 0,-1), vec4(-1, 0, 0,-1), vec4( 0, 1, 1, 0), vec4( 0,-1, 1, 0), vec4( 0, 1,-1, 0), vec4( 0,-1,-1, 0), vec4( 0, 1, 0, 1), vec4( 0,-1, 0, 1), vec4( 0, 1, 0,-1), vec4( 0,-1, 0,-1), vec4( 0, 0, 1, 1), vec4( 0, 0,-1, 1), vec4( 0, 0, 1,-1), vec4( 0, 0,-1,-1)};
const std::vector<vec4> unicorn_delta = {vec4( 1, 1, 1, 0), vec4(-1, 1, 1, 0), vec4( 1,-1, 1, 0), vec4(-1,-1, 1, 0), vec4( 1, 1,-1, 0), vec4(-1, 1,-1, 0), vec4( 1,-1,-1, 0), vec4(-1,-1,-1, 0), vec4( 1, 1, 0, 1), vec4(-1, 1, 0, 1), vec4( 1,-1, 0, 1), vec4(-1,-1, 0, 1), vec4( 1, 1, 0,-1), vec4(-1, 1, 0,-1), vec4( 1,-1, 0,-1), vec4(-1,-1, 0,-1), vec4( 1, 0, 1, 1), vec4(-1, 0, 1, 1), vec4( 1, 0,-1, 1), vec4(-1, 0,-1, 1), vec4( 1, 0, 1,-1), vec4(-1, 0, 1,-1), vec4( 1, 0,-1,-1), vec4(-1, 0,-1,-1), vec4( 0, 1, 1, 1), vec4( 0,-1, 1, 1), vec4( 0, 1,-1, 1), vec4( 0,-1,-1, 1), vec4( 0, 1, 1,-1), vec4( 0,-1, 1,-1), vec4( 0, 1,-1,-1), vec4( 0,-1,-1,-1)};
const std::vector<vec4> dragon_delta = {vec4( 1, 1, 1, 1), vec4(-1, 1, 1, 1), vec4( 1,-1, 1, 1), vec4(-1,-1, 1, 1), vec4( 1, 1,-1, 1), vec4(-1, 1,-1, 1), vec4( 1,-1,-1, 1), vec4(-1,-1,-1, 1), vec4( 1, 1, 1,-1), vec4(-1, 1, 1,-1), vec4( 1,-1, 1,-1), vec4(-1,-1, 1,-1), vec4( 1, 1,-1,-1), vec4(-1, 1,-1,-1), vec4( 1,-1,-1,-1), vec4(-1,-1,-1,-1)};
const std::vector<vec4> princess_delta = concat_vectors(rook_delta, bishop_delta);
const std::vector<vec4> queen_delta = concat_vectors(rook_delta, bishop_delta, unicorn_delta, dragon_delta);
const std::vector<vec4> pawn_unmoved_white_delta = {vec4( 0, 2, 0, 0), vec4( 0, 0, 0,-2)};
const std::vector<vec4> pawn_white_delta = {vec4( 0, 1, 0, 0), vec4( 0, 0, 0,-1)};
const std::vector<vec4> pawn_white_capture_delta = {vec4( 1, 1, 0, 0), vec4(-1, 1, 0, 0), vec4( 0, 0, 1,-1), vec4( 0, 0,-1,-1)};
const std::vector<vec4> pawn_unmoved_black_delta = {vec4( 0,-2, 0, 0), vec4( 0, 0, 0, 2)};
const std::vector<vec4> pawn_black_delta = {vec4( 0,-1, 0, 0), vec4( 0, 0, 0, 1)};
const std::vector<vec4> pawn_black_capture_delta = {vec4( 1,-1, 0, 0), vec4(-1,-1, 0, 0), vec4( 0, 0, 1, 1), vec4( 0, 0,-1, 1)};

namespace views = std::ranges::views;

std::vector<vec4> multiverse::gen_piece_move(const vec4& p, int board_color) const
{
    std::shared_ptr<board> b_ptr = get_board(p.l(), p.t(), board_color);
    piece_t p_piece = get_piece(p, board_color);
    const bool p_color = piece_color(p_piece);
    if(get_umove_flag(p, board_color))
    {
        p_piece = static_cast<piece_t>(p_piece | 0x80);
    }
    std::vector<vec4> result;
    auto is_blank = [&](vec4 d)
    {
        piece_t q_piece = get_piece(p+d, board_color);
        return q_piece == NO_PIECE;
    };
    auto can_go_to = [&](vec4 d)
    {
        piece_t q_piece = get_piece(p+d, board_color);
        return q_piece == NO_PIECE || p_color != piece_color(q_piece);
    };
    auto delta_in_range = [&](vec4 d)
    {
        return inbound(p+d, board_color);
    };
    auto good_castling_direction = [&](vec4 d)
    {
        int i = 0;
        for(vec4 c = d; delta_in_range(c); c = c + d)
        {
            vec4 q = p+c;
            piece_t q_piece = get_piece(q, board_color);
            if(get_umove_flag(q, board_color))
            {
                q_piece = static_cast<piece_t>(q_piece | 0x80);
            }
            if(i < 2 && b_ptr->is_under_attack(q.xy(), board_color))
            {
                return false;
            }
            else if(q_piece == ROOK_UW || q_piece == ROOK_UB)
            {
                return true;
            }
            else if(q_piece != NO_PIECE)
            {
                return false;
            }
            i++;
        }
        return false;
    };
    auto multiple_moves = [&](const std::vector<vec4>& delta)
    {
        std::vector<vec4> res;
        //I wold prefer to write `vector<tuple<vec4,vec4>>` instead of `auto`, but clang++ and visual studio complain about different type errors
        auto zipped_delta = std::views::zip(std::vector<vec4>(delta.size(), vec4(0,0,0,0)), delta) | std::ranges::to<std::vector>();
        while(!zipped_delta.empty())
        {
            for(std::tuple<vec4&, vec4> m : zipped_delta)
            {
                std::get<0>(m) = std::get<0>(m) + std::get<1>(m);
            }
            zipped_delta = zipped_delta | views::filter([&](auto x){
                vec4 d = std::get<0>(x);
                return delta_in_range(d);
            }) | std::ranges::to<std::vector>();
            //print_range("after checking range:", zipped_delta);
            auto tmp = zipped_delta
            | views::transform([](auto elem){return std::get<0>(elem);})
            | views::filter(can_go_to)
            | std::ranges::to<std::vector>();
            //print_range("tmp:", tmp);
            append_vectors(res, tmp);
            zipped_delta = zipped_delta | views::filter([&](auto x){
                vec4 d = std::get<0>(x);
                return is_blank(d);
            }) | std::ranges::to<std::vector>();
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
            | std::ranges::to<std::vector>();
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
        case PRINCESS_W:
        case PRINCESS_B:
            result = multiple_moves(princess_delta);
        case ROYAL_QUEEN_W:
        case ROYAL_QUEEN_B:
            if(!b_ptr->is_under_attack(p.xy(), board_color))
            {
                result = std::vector<vec4>{vec4(-1,0,0,0), vec4(1,0,0,0)}
                | views::filter(good_castling_direction)
                | views::transform([](vec4 v){return v+v;})
                | std::ranges::to<std::vector>();
            }
        case QUEEN_W:
        case QUEEN_B:
            append_vectors(result, multiple_moves(queen_delta));
            break;
        case KING_UW:
        case KING_UB:
            if(!b_ptr->is_under_attack(p.xy(), board_color))
            {
                result = std::vector<vec4>{vec4(-1,0,0,0), vec4(1,0,0,0)}
                | views::filter(good_castling_direction)
                | views::transform([](vec4 v){return v+v;})
                | std::ranges::to<std::vector>();
            }
        case COMMON_KING_W:
        case COMMON_KING_B:
        case KING_W:
        case KING_B:
            append_vectors(result, queen_delta
            | views::filter(delta_in_range)
            | views::filter(can_go_to)
            | std::ranges::to<std::vector>());
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
                if(q_name != NO_PIECE && p_color!=piece_color(q_name))
                {
                    result.push_back(d);
                }
                // en passant
                else if(q_name == NO_PIECE
                    && get_piece(p+vec4(d.x(),0,0,0), board_color) == PAWN_B
                    && inbound(p+vec4(d.x(),2,-1,0), board_color)
                    && get_piece(p+vec4(d.x(),0,-1,0), board_color) == NO_PIECE
                    && get_piece(p+vec4(d.x(),2,-1,0), board_color) == PAWN_B
                    && get_umove_flag(p+vec4(d.x(),2,-1,0), board_color))
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
                if(q_name != NO_PIECE && p_color!=piece_color(q_name))
                {
                    result.push_back(d);
                }
                // en passant
                else if(q_name == NO_PIECE && get_piece(p+vec4(d.x(),0,0,0), board_color) == PAWN_W
                    && inbound(p+vec4(d.x(),-2,-1,0), board_color)
                    && get_piece(p+vec4(d.x(),0,-1,0), board_color) == NO_PIECE
                    && get_piece(p+vec4(d.x(),-2,-1,0), board_color) == PAWN_UW
                    && get_umove_flag(p+vec4(d.x(),-2,-1,0), board_color))
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

#else //USE_LEAGACY_GENMOVE



/************************************
 
 *
 **
 ***
 *****
 ******
 *******
 */

std::vector<vec4> multiverse::gen_piece_move(const vec4& p, int board_color) const
{
    std::shared_ptr<board> b_ptr = get_board(p.l(), p.t(), board_color);
    piece_t p_piece = get_piece(p, board_color);
    const bool p_color = piece_color(p_piece);
    if(get_umove_flag(p, board_color))
    {
        p_piece = static_cast<piece_t>(p_piece | 0x80);
    }
    std::vector<multiverse::tagged_bb> mvbbs;
    switch(p_piece)
    {
        case KING_W:
            mvbbs = gen_superphysical_move<KING_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<KING_W, 0>(p)));
            break;
        case KING_B:
            mvbbs = gen_superphysical_move<KING_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<KING_B, 1>(p)));
            break;
        case COMMON_KING_W:
            mvbbs = gen_superphysical_move<COMMON_KING_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<COMMON_KING_W, 0>(p)));
            break;
        case COMMON_KING_B:
            mvbbs = gen_superphysical_move<COMMON_KING_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<COMMON_KING_B, 1>(p)));
            break;
        case ROOK_W:
            mvbbs = gen_superphysical_move<ROOK_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<ROOK_W, 0>(p)));
            break;
        case ROOK_B:
            mvbbs = gen_superphysical_move<ROOK_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<ROOK_B, 1>(p)));
            break;
        case BISHOP_W:
            mvbbs = gen_superphysical_move<BISHOP_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<BISHOP_W, 0>(p)));
            break;
        case BISHOP_B:
            mvbbs = gen_superphysical_move<BISHOP_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<BISHOP_B, 1>(p)));
            break;
        case QUEEN_W:
            mvbbs = gen_superphysical_move<QUEEN_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<QUEEN_W, 0>(p)));
            break;
        case QUEEN_B:
            mvbbs = gen_superphysical_move<QUEEN_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<QUEEN_B, 1>(p)));
            break;
        case PRINCESS_W:
            mvbbs = gen_superphysical_move<PRINCESS_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<PRINCESS_W, 0>(p)));
            break;
        case PRINCESS_B:
            mvbbs = gen_superphysical_move<PRINCESS_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<PRINCESS_B, 1>(p)));
            break;
        case PAWN_W:
            mvbbs = gen_superphysical_move<PAWN_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<PAWN_W, 0>(p)));
            break;
        case BRAWN_W:
            mvbbs = gen_superphysical_move<BRAWN_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<BRAWN_W, 0>(p)));
            break;
        case PAWN_B:
            mvbbs = gen_superphysical_move<PAWN_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<PAWN_B, 1>(p)));
            break;
        case BRAWN_B:
            mvbbs = gen_superphysical_move<BRAWN_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<BRAWN_B, 1>(p)));
            break;
        case PAWN_UW:
            mvbbs = gen_superphysical_move<PAWN_UW, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<PAWN_UW, 0>(p)));
            break;
        case BRAWN_UW:
            mvbbs = gen_superphysical_move<BRAWN_UW, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<BRAWN_UW, 0>(p)));
            break;
        case PAWN_UB:
            mvbbs = gen_superphysical_move<PAWN_UB, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<PAWN_UB, 1>(p)));
            break;
        case BRAWN_UB:
            mvbbs = gen_superphysical_move<BRAWN_UB, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<BRAWN_UB, 1>(p)));
            break;
        case KNIGHT_W:
            mvbbs = gen_superphysical_move<KNIGHT_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<KNIGHT_W, 0>(p)));
            break;
        case KNIGHT_B:
            mvbbs = gen_superphysical_move<KNIGHT_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<KNIGHT_B, 1>(p)));
            break;
        case UNICORN_W:
            mvbbs = gen_superphysical_move<UNICORN_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<UNICORN_W, 0>(p)));
            break;
        case UNICORN_B:
            mvbbs = gen_superphysical_move<UNICORN_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<UNICORN_B, 1>(p)));
            break;
        case DRAGON_W:
            mvbbs = gen_superphysical_move<DRAGON_W, 0>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<DRAGON_W, 0>(p)));
            break;
        case DRAGON_B:
            mvbbs = gen_superphysical_move<DRAGON_B, 1>(p);
            mvbbs.push_back(std::make_tuple(p, gen_physical_move<DRAGON_B, 1>(p)));
            break;
        default:
            throw std::runtime_error("Unknown piece" + std::to_string(piece_name(p_piece)) + (p_piece & 0x80 ? "*": "") + "\n");
            // Handle unknown piece or add error handling if needed
            break;
    }
    std::vector<vec4> result;
    for (const auto& [r, bb] : mvbbs)
    {
        for(int pos : marked_pos(bb))
        {
            vec4 q = vec4(pos&((1<<BOARD_BITS)-1), pos>>BOARD_BITS, r.t(), r.l()) - p;
			//std::cerr << q << std::endl;
            result.push_back(q);
        }
    }
    return result;
}

#endif //USE_LEAGACY_GENMOVE


constexpr std::initializer_list<vec4> orthogonal_dtls = {
    vec4(0, 0, 0, 1),
    vec4(0, 0, 0, -1),
    vec4(0, 0, -1, 0)
};

constexpr std::initializer_list<vec4> diagonal_dtls = {
    vec4(0, 0, 1, 1),
    vec4(0, 0, 1, -1),
    vec4(0, 0, -1, 1),
    vec4(0, 0, -1, -1)
};

constexpr std::initializer_list<vec4> both_dtls = {
    vec4(0, 0, 0, 1),
    vec4(0, 0, 0, -1),
    vec4(0, 0, -1, 0),
    vec4(0, 0, 1, 1),
    vec4(0, 0, 1, -1),
    vec4(0, 0, -1, 1),
    vec4(0, 0, -1, -1)
};

constexpr std::initializer_list<vec4> double_dtls = {
    vec4(0, 0, 0, 2),
    vec4(0, 0, 0, -2),
    vec4(0, 0, -2, 0)
};

template<bool C>
std::vector<multiverse::tagged_bb> multiverse::gen_purely_sp_rook_move(vec4 p0) const
{
    std::vector<tagged_bb> result;
    std::shared_ptr<board> b0_ptr = get_board(p0.l(), p0.t(), C);
    bitboard_t lrook;
    if constexpr(C)
    {
        lrook = b0_ptr->lrook() & b0_ptr->black();
    }
    else
    {
        lrook = b0_ptr->lrook() & b0_ptr->white();
    }
    for(auto delta : orthogonal_dtls)
    {
        bitboard_t remaining = lrook;
        for(vec4 p1 = p0; remaining && inbound(p1, C); p1=p1+delta)
        {
            std::shared_ptr<board> b1_ptr = get_board(p1.l(), p1.t(), C);
            bitboard_t friendly, hostile;
            if constexpr(C)
            {
                friendly = b1_ptr->black();
                hostile = b1_ptr->white();
            }
            else
            {
                friendly = b1_ptr->white();
                hostile = b1_ptr->black();
            }
            remaining &= ~friendly;
            if(remaining)
            {
                result.push_back(std::make_tuple(p1, remaining));
                remaining &= ~hostile;
            }
        }
    }
    return result;
}


template<bool C>
std::vector<multiverse::tagged_bb> multiverse::gen_purely_sp_bishop_move(vec4 p0) const
{
    std::vector<tagged_bb> result;
    std::shared_ptr<board> b0_ptr = get_board(p0.l(), p0.t(), C);
    bitboard_t lbishop;
    if constexpr(C)
    {
        lbishop = b0_ptr->lbishop() & b0_ptr->black();
    }
    else
    {
        lbishop = b0_ptr->lbishop() & b0_ptr->white();
    }
    for(auto delta : diagonal_dtls)
    {
        bitboard_t remaining = lbishop;
        for(vec4 p1 = p0; remaining && inbound(p1, C); p1=p1+delta)
        {
            std::shared_ptr<board> b1_ptr = get_board(p1.l(), p1.t(), C);
            bitboard_t friendly, hostile;
            if constexpr(C)
            {
                friendly = b1_ptr->black();
                hostile = b1_ptr->white();
            }
            else
            {
                friendly = b1_ptr->white();
                hostile = b1_ptr->black();
            }
            remaining &= ~friendly;
            if(remaining)
            {
                result.push_back(std::make_tuple(p1, remaining));
                remaining &= ~hostile;
            }
        }
    }
    return result;
}


template<bool C>
std::vector<multiverse::tagged_bb> multiverse::gen_purely_sp_knight_move(vec4 p0) const
{
    std::vector<tagged_bb> result;
    std::shared_ptr<board> b0_ptr = get_board(p0.l(), p0.t(), C);
    bitboard_t lknight;
    if constexpr(C)
    {
        lknight = b0_ptr->lknight() & b0_ptr->black();
    }
    else
    {
        lknight = b0_ptr->lknight() & b0_ptr->white();
    }
    const static std::vector<vec4> knight_pure_sp_delta = {vec4(0, 0, 2, 1), vec4(0, 0, 1, 2), vec4(0, 0, -2, 1), vec4(0, 0, 1, -2),
        vec4(0, 0, 2, -1), vec4(0, 0, -1, 2), vec4(0, 0, -2, -1), vec4(0, 0, -1, -2)};
    for(vec4 delta : knight_pure_sp_delta)
    {
        vec4 p1 = p0 + delta;
        if (inbound(p1, C))
        {
            std::shared_ptr<board> b1_ptr = get_board(p1.l(), p1.t(), C);
            bitboard_t remaining = lknight;
            bitboard_t friendly;
            if constexpr (C)
            {
                friendly = b1_ptr->black();
            }
            else
            {
                friendly = b1_ptr->white();
            }
            remaining &= ~friendly;
            if (remaining)
            {
                result.push_back(std::make_tuple(p1, remaining));
            }
        }
    }
    return result;
}


template<piece_t P, bool C>
bitboard_t multiverse::gen_physical_move(vec4 p) const
{
	//int u = l_to_u(p.l()), v = tc_to_v(p.t(), board_color);
	std::shared_ptr<board> b_ptr = get_board(p.l(), p.t(), C);
    bitboard_t friendly = b_ptr->friendly<C>();
    bitboard_t hostile = b_ptr->hostile<C>();
    bitboard_t a;
    int pos = p.xy();
    bitboard_t z = pmask(pos);
    bitboard_t empty = ~(friendly | hostile);
    if constexpr (P == KING_W || P == KING_B || P == COMMON_KING_W || P == COMMON_KING_B)
    {
        a = king_attack(p.xy()) & ~friendly;
    }
    else if (P == KING_UW || P == KING_UB)
    {
        a = king_attack(p.xy()) & ~friendly;
        bitboard_t urook = b_ptr->umove() & b_ptr->rook() & friendly;
        if(!b_ptr->is_under_attack(p.xy(), C))
        {
            for(vec4 d : {vec4(1,0,0,0), vec4(-1,0,0,0)})
            {
                int i = 0;
                for(vec4 c = d; !c.outbound(); c = c + d)
                {
                    vec4 q = p+c;
                    bitboard_t w = pmask(q.xy());
                    if(i < 2 && b_ptr->is_under_attack(q.xy(), C))
                    {
                        return false;
                    }
                    else if(w & b_ptr->umove() & b_ptr->rook() & friendly)
                    {
                        return true;
                    }
                    else if(w & b_ptr->occupied())
                    {
                        return false;
                    }
                    i++;
                }
            }
        }
    }
    else if (P == ROOK_W || P == ROOK_B)
    {
		a = rook_attack(p.xy(), b_ptr->occupied()) & ~friendly;
	}
    else if (P == BISHOP_W || P == BISHOP_B)
    {
		a = bishop_attack(p.xy(), b_ptr->occupied()) & ~friendly;
    }
    else if (P == QUEEN_W || P == QUEEN_B || P == PRINCESS_W || P == PRINCESS_B)
    {
        a = queen_attack(p.xy(), b_ptr->occupied()) & ~friendly;
	}
    else if (P == PAWN_W || P == BRAWN_W || P == PAWN_UW || P == BRAWN_UW)
    {
        bitboard_t patt = white_pawn_attack(pos);
        // normal move and capture
		a = (patt & hostile) | (shift_north(z) & empty);
        // en passant
        bitboard_t r = (shift_west(z) | shift_east(z)) & hostile & b_ptr->pawn();
        bitboard_t s = shift_north(shift_north(r)) & empty;
        if(s)
        {
            vec4 q = p+vec4(0, 2, -1, 0);
            if(inbound(q, C))
            {
                std::shared_ptr<board> b1_ptr = get_board(q.l(), q.t(), C);
                bitboard_t j = s & b1_ptr->umove() & ~friendly & b1_ptr->pawn();
                a |= shift_south(j);
            }
        }
        // additional move for unmoved pawns
        if constexpr (P == PAWN_UW || P == BRAWN_UW)
        {
            a |= shift_north(shift_north(z) & empty);
        }
    }
    else if (P == PAWN_B || P == BRAWN_B || P == PAWN_UB || P == BRAWN_UB)
    {
        bitboard_t patt = black_pawn_attack(pos);
        // normal move and capture
        a = (patt & hostile) | (shift_south(z) & empty);
        // en passant
        bitboard_t r = (shift_west(z) | shift_east(z)) & hostile & b_ptr->pawn();
        bitboard_t s = shift_south(shift_south(r)) & empty;
        if(s)
        {
            vec4 q = p+vec4(0, 2, -1, 0);
            if(inbound(q, C))
            {
                std::shared_ptr<board> b1_ptr = get_board(q.l(), q.t(), C);
                bitboard_t j = s & b1_ptr->umove() & ~friendly & b1_ptr->pawn();
                a |= shift_north(j);
            }
        }
        // additional move for unmoved pawns
        if constexpr (P == PAWN_UB || P == BRAWN_UB)
        {
            a |= shift_south(shift_south(z) & empty);
        }
    }
	else if (P == KNIGHT_W || P == KNIGHT_B)
	{
		a = knight_attack(p.xy()) & ~friendly;
	}
    else if (P == UNICORN_W || P == UNICORN_B || P == DRAGON_W || P == DRAGON_B)
    {
        a = 0;
    }
	else
	{
		std::cerr << "gen_physical_move:" << P << "not implemented" << std::endl;
	}
	//std::cerr << "gen_physical_move:" << piece_name(P) << " " << p << std::endl;
	//std::cerr << bb_to_string(a) << std::endl;
	return a;
}


template<bool C, multiverse::axesmode TL, multiverse::axesmode XY>
void multiverse::gen_compound_moves(vec4 p, std::vector<multiverse::tagged_bb>& result) const
{
    int pos = p.xy();
    bitboard_t occ = 0;
    bitboard_t fri = 0;
    bitboard_t copy_mask;
    
    constexpr auto deltas = (TL==multiverse::axesmode::ORTHOGONAL) ? orthogonal_dtls : (TL==multiverse::axesmode::DIAGONAL) ? diagonal_dtls : both_dtls;
    
    constexpr auto copy_mask_fn = (TL==multiverse::axesmode::ORTHOGONAL) ? rook_copy_mask : (TL==multiverse::axesmode::DIAGONAL) ? bishop_copy_mask : queen_copy_mask;

    for(vec4 d : deltas)
    {
        vec4 q = p;
        for (int n = 1; (copy_mask = copy_mask_fn(pos, 1)); n++)
        {
            q = q + d;
            if(inbound(q, C))
            {
                std::shared_ptr<board> b_ptr = get_board(q.l(), q.t(), C);
                occ |= copy_mask & b_ptr->occupied();
                fri |= copy_mask & b_ptr->friendly<C>();
            }
        }
        bitboard_t loc = ~fri;
        if constexpr (XY == multiverse::axesmode::ORTHOGONAL)
        {
            loc &= rook_attack(pos, occ);
        }
        else if (XY == multiverse::axesmode::DIAGONAL)
        {
            loc &= bishop_attack(pos, occ);
        }
        else
        {
            loc &= queen_attack(pos, occ);
        }
        q = p;
        for (int n = 1; (copy_mask = bishop_copy_mask(pos, 1)); n++) 
        {
            q = q + d;
            bitboard_t c = loc & copy_mask;
            if(c) 
            {
                result.push_back(std::make_tuple(q, c));
            } 
            else 
            {
                break;
            }
        }
    }
}

template<piece_t P, bool C>
std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move(vec4 p) const
{
    std::vector<multiverse::tagged_bb> result;
    int l = p.l(), t = p.t(), pos = p.xy();
    if constexpr (P == KING_W || P == KING_B || P == COMMON_KING_W || P == COMMON_KING_B)
    {
        for(auto d : both_dtls)
        {
            vec4 q = p+d;
            if(inbound(q, C))
            {
                std::shared_ptr<board> b_ptr = get_board(q.l(), q.t(), C);
                bitboard_t bb = king_jump_attack(pos) & ~b_ptr->friendly<C>();
                if(bb)
                {
                    result.push_back(std::make_tuple(q, bb));
                }
            }
        }
    }
    else if (P == ROOK_W || P == ROOK_B)
    {
        bitboard_t z = pmask(p.xy());
        for(auto [index, bb] : gen_purely_sp_rook_move<C>(p))
        {
            bitboard_t bb1 = bb & z;
            if(bb1)
            {
                result.push_back(std::make_tuple(index, bb1));
            }
        }
    }
    else if (P == BISHOP_W || P == BISHOP_B)
    {
        bitboard_t z = pmask(p.xy());
        for(auto [index, bb] : gen_purely_sp_rook_move<C>(p))
        {
            bitboard_t bb1 = bb & z;
            if(bb1)
            {
                result.push_back(std::make_tuple(index, bb1));
            }
        }
        gen_compound_moves<C, multiverse::axesmode::ORTHOGONAL, multiverse::axesmode::ORTHOGONAL>(p, result);
    }
    else if (P == PRINCESS_W || P == PRINCESS_B)
    {
        result = concat_vectors(gen_superphysical_move<ROOK_W,C>(p),
                                gen_superphysical_move<BISHOP_W,C>(p));
    }
    else if (P == QUEEN_W || P == QUEEN_B)
    {
        bitboard_t z = pmask(p.xy());
        for(auto [index, bb] : gen_purely_sp_rook_move<C>(p))
        {
            bitboard_t bb1 = bb & z;
            if(bb1)
            {
                result.push_back(std::make_tuple(index, bb1));
            }
        }
        for(auto [index, bb] : gen_purely_sp_bishop_move<C>(p))
        {
            bitboard_t bb1 = bb & z;
            if(bb1)
            {
                result.push_back(std::make_tuple(index, bb1));
            }
        }
        gen_compound_moves<C, multiverse::axesmode::BOTH, multiverse::axesmode::BOTH>(p, result);
        
    }
    else if (P == PAWN_W || P == BRAWN_W || P == PAWN_UW || P == BRAWN_UW)
    {
        bitboard_t z = pmask(pos);
        // pawn capture
        static std::vector<vec4> pawn_w_cap_tl_delta = {vec4(0, 0, 1, 1), vec4(0, 0, -1, 1)};
        for(vec4 d : pawn_w_cap_tl_delta)
        {
            vec4 q = p + d;
            if(inbound(q, C))
            {
                std::shared_ptr<board> b_ptr = get_board(q.l(), q.t(), C);
                bitboard_t bb = z & b_ptr->hostile<C>();
                if(bb)
                {
                    result.push_back(std::make_tuple(q, bb));
                }
            }
        }
        // normal pawn movement -- bitboard saved in the very end of the if block
        vec4 q = p + vec4(0,0,0,1);
        if(inbound(q, C))
        {
            std::shared_ptr<board> b_ptr = get_board(q.l(), q.t(), C);
            bitboard_t bb = z & ~b_ptr->occupied();
            if(bb)
            {
                // unmoved pawn movement
                if constexpr(P == PAWN_UW || P == BRAWN_UW)
                {
                    vec4 r = q + vec4(0,0,0,1);
                    std::shared_ptr<board> b1_ptr = get_board(r.l(), r.t(), C);
                    bitboard_t bc = z & ~b_ptr->occupied();
                    if(bc)
                    {
                        result.push_back(std::make_tuple(r, bc));
                    }
                }
            }
            // brawn capture
            if constexpr(P == BRAWN_W || P == BRAWN_UW)
            {
                bitboard_t mask = shift_north(z) | shift_west(z) | shift_east(z);
                bb |= mask & b_ptr->hostile<C>();
            }
            if(bb)
            {
                result.push_back(std::make_tuple(q, bb));
            }
        }
        if constexpr(P == BRAWN_W || P == BRAWN_UW)
        {
            vec4 s = p + vec4(0,1,-1,0);
            if(inbound(s, C))
            {
                std::shared_ptr<board> b2_ptr = get_board(s.l(), s.t(), C);
                bitboard_t bd = shift_north(z) & ~b2_ptr->occupied();
                if(bd)
                {
                    result.push_back(std::make_tuple(s, bd));
                }
            }
        }
    }
    else if (P == PAWN_B || P == BRAWN_B || P == PAWN_UB || P == BRAWN_UB)
    {
        bitboard_t z = pmask(pos);
        // pawn capture
        static std::vector<vec4> pawn_w_cap_tl_delta = {vec4(0, 0, 1, -1), vec4(0, 0, -1, -1)};
        for(vec4 d : pawn_w_cap_tl_delta)
        {
            vec4 q = p + d;
            if(inbound(q, C))
            {
                std::shared_ptr<board> b_ptr = get_board(q.l(), q.t(), C);
                bitboard_t bb = z & b_ptr->hostile<C>();
                if(bb)
                {
                    result.push_back(std::make_tuple(q, bb));
                }
            }
        }
        // normal pawn movement -- bitboard saved in the very end of the if block
        vec4 q = p + vec4(0,0,0,-1);
        if(inbound(q, C))
        {
            std::shared_ptr<board> b_ptr = get_board(q.l(), q.t(), C);
            bitboard_t bb = z & ~b_ptr->occupied();
            if(bb)
            {
                // unmoved pawn movement
                if constexpr(P == PAWN_UW || P == BRAWN_UW)
                {
                    vec4 r = q + vec4(0,0,0,-1);
                    std::shared_ptr<board> b1_ptr = get_board(r.l(), r.t(), C);
                    bitboard_t bc = z & ~b_ptr->occupied();
                    if(bc)
                    {
                        result.push_back(std::make_tuple(r, bc));
                    }
                }
            }
            // brawn capture
            if constexpr(P == BRAWN_W || P == BRAWN_UW)
            {
                bitboard_t mask = shift_south(z) | shift_west(z) | shift_east(z);
                bb |= mask & b_ptr->hostile<C>();
            }
            if(bb)
            {
                result.push_back(std::make_tuple(q, bb));
            }
        }
        if constexpr(P == BRAWN_W || P == BRAWN_UW)
        {
            vec4 s = p + vec4(0,1,-1,0);
            if(inbound(s, C))
            {
                std::shared_ptr<board> b2_ptr = get_board(s.l(), s.t(), C);
                bitboard_t bd = shift_north(z) & ~b2_ptr->occupied();
                if(bd)
                {
                    result.push_back(std::make_tuple(s, bd));
                }
            }
        }
    }
    else if (P == KNIGHT_W || P == KNIGHT_B)
    {
        for(auto [index, bb] : gen_purely_sp_knight_move<C>(p))
        {
            bitboard_t bb1 = bb & pmask(pos);
            if(bb1)
            {
                result.push_back(std::make_tuple(index, bb1));
            }
        }
        for(auto d : orthogonal_dtls)
        {
            vec4 q = p+d;
            if(inbound(q, C))
            {
                std::shared_ptr<board> b_ptr = get_board(q.l(), q.t(), C);
                bitboard_t bb = knight_jump1_attack(pos) & ~b_ptr->friendly<C>();
                if(bb)
                {
                    result.push_back(std::make_tuple(q, bb));
                }
            }
        }
        for(auto d : double_dtls)
        {
            vec4 q = p+d;
            if(inbound(q, C))
            {
                std::shared_ptr<board> b_ptr = get_board(q.l(), q.t(), C);
                bitboard_t bb = knight_jump2_attack(pos) & ~b_ptr->friendly<C>();
                if(bb)
                {
                    result.push_back(std::make_tuple(q, bb));
                }
            }
        }
    }
    else if (P == UNICORN_W || P == UNICORN_B)
    {
        gen_compound_moves<C, multiverse::axesmode::ORTHOGONAL, multiverse::axesmode::DIAGONAL>(p, result);
        gen_compound_moves<C, multiverse::axesmode::DIAGONAL, multiverse::axesmode::ORTHOGONAL>(p, result);
    }
    else if (P == DRAGON_W || P == DRAGON_B)
    {
        gen_compound_moves<C, multiverse::axesmode::DIAGONAL, multiverse::axesmode::DIAGONAL>(p, result);
    }
    else
    {
        std::cerr << "gen_superphysical_move:" << P << "not implemented" << std::endl;
    }
    return result;
}


// Explicit instantiation of the template for specific types
template bitboard_t multiverse::gen_physical_move<KING_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<KING_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<COMMON_KING_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<COMMON_KING_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<ROOK_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<ROOK_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<BISHOP_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<BISHOP_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<QUEEN_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<QUEEN_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PRINCESS_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PRINCESS_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PAWN_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PAWN_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PAWN_UW, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PAWN_UB, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<KNIGHT_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<KNIGHT_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<UNICORN_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<UNICORN_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<DRAGON_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<DRAGON_B, true>(vec4 p) const;

template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<KING_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<KING_B, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<COMMON_KING_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<COMMON_KING_B, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<ROOK_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<ROOK_B, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<BISHOP_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<BISHOP_B, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<QUEEN_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<QUEEN_B, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<PRINCESS_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<PRINCESS_B, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<PAWN_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<PAWN_B, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<PAWN_UW, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<PAWN_UB, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<KNIGHT_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<KNIGHT_B, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<UNICORN_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<UNICORN_B, true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<DRAGON_W, false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_superphysical_move<DRAGON_B, true>(vec4 p) const;

template std::vector<multiverse::tagged_bb> multiverse::gen_purely_sp_rook_move<false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_purely_sp_rook_move<true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_purely_sp_bishop_move<false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_purely_sp_bishop_move<true>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_purely_sp_knight_move<false>(vec4 p) const;
template std::vector<multiverse::tagged_bb> multiverse::gen_purely_sp_knight_move<true>(vec4 p) const;

template void multiverse::gen_compound_moves<false, multiverse::axesmode::ORTHOGONAL, multiverse::axesmode::ORTHOGONAL>(vec4 p, std::vector<multiverse::tagged_bb>& result) const;
template void multiverse::gen_compound_moves<true, multiverse::axesmode::ORTHOGONAL, multiverse::axesmode::ORTHOGONAL>(vec4 p, std::vector<multiverse::tagged_bb>& result) const;
template void multiverse::gen_compound_moves<false, multiverse::axesmode::DIAGONAL, multiverse::axesmode::DIAGONAL>(vec4 p, std::vector<multiverse::tagged_bb>& result) const;
template void multiverse::gen_compound_moves<true, multiverse::axesmode::DIAGONAL, multiverse::axesmode::DIAGONAL>(vec4 p, std::vector<multiverse::tagged_bb>& result) const;
template void multiverse::gen_compound_moves<false, multiverse::axesmode::BOTH, multiverse::axesmode::BOTH>(vec4 p, std::vector<multiverse::tagged_bb>& result) const;
template void multiverse::gen_compound_moves<true, multiverse::axesmode::BOTH, multiverse::axesmode::BOTH>(vec4 p, std::vector<multiverse::tagged_bb>& result) const;
template bitboard_t multiverse::gen_physical_move<PRINCESS_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PRINCESS_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PAWN_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PAWN_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PAWN_UW, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<PAWN_UB, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<KNIGHT_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<KNIGHT_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<UNICORN_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<UNICORN_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<DRAGON_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_move<DRAGON_B, true>(vec4 p) const;
