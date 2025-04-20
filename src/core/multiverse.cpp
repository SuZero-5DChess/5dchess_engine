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


multiverse::multiverse(const std::string &input,  int size_x, int size_y)
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
            insert_board(l, t, c, std::make_shared<board>(sm[1], size_x, size_y));
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

multiverse::multiverse(const multiverse& other)
    : boards(other.boards),
      sp_rook_moves_w(), sp_rook_moves_b(),
      sp_bishop_moves_w(), sp_bishop_moves_b(),
      sp_knight_moves_w(), sp_knight_moves_b(),
      all_moves_w(), all_moves_b(),
      l_min(other.l_min),
      l_max(other.l_max),
      timeline_start(other.timeline_start),
      timeline_end(other.timeline_end)
{}

multiverse& multiverse::operator=(const multiverse& other)
{
    if (this != &other)
    {
        boards = other.boards;
        clear_cache();
    }
    return *this;
}

void multiverse::clear_cache() const
{
    sp_rook_moves_w.clear();
    sp_bishop_moves_w.clear();
    sp_knight_moves_w.clear();
    sp_rook_moves_b.clear();
    sp_bishop_moves_b.clear();
    sp_knight_moves_b.clear();
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

std::shared_ptr<board> multiverse::get_board(int l, int t, int c) const
{
    try
    {
        return this->boards.at(l_to_u(l)).at(tc_to_v(t,c));
    }
    catch(const std::out_of_range& ex)
    {
		std::cerr << ex.what() << std::endl;
        std::cerr << "In this multiverse object:\n" << to_string();
        std::cerr << "Error: Out of range in multiverse::get_board("
        << l << ", " << t << ", " << c << ")"<< std::endl;
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


/************************************
 
 *
 **
 ***
 *****
 ******
 *******
 */

template<bool C>
std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves(vec4 p) const
{
    std::shared_ptr<board> b_ptr = get_board(p.l(), p.t(), C);
    piece_t p_piece = b_ptr->get_piece(p.xy());
    if (b_ptr->umove() & pmask(p.xy()))
    {
        p_piece = static_cast<piece_t>(p_piece | 0x80);
    }
    std::map<vec4, bitboard_t> mvbbs;
    switch (p_piece)
    {
#define GENERATE_MOVES_CASE(PIECE) \
        case PIECE: \
            mvbbs = gen_superphysical_moves_impl<PIECE, C>(p); \
            break;

        GENERATE_MOVES_CASE(KING_W)
        GENERATE_MOVES_CASE(KING_B)
        GENERATE_MOVES_CASE(KING_UW)
        GENERATE_MOVES_CASE(KING_UB)
        GENERATE_MOVES_CASE(COMMON_KING_W)
        GENERATE_MOVES_CASE(COMMON_KING_B)
        GENERATE_MOVES_CASE(ROOK_W)
        GENERATE_MOVES_CASE(ROOK_B)
        GENERATE_MOVES_CASE(ROOK_UW)
        GENERATE_MOVES_CASE(ROOK_UB)
        GENERATE_MOVES_CASE(BISHOP_W)
        GENERATE_MOVES_CASE(BISHOP_B)
        GENERATE_MOVES_CASE(QUEEN_W)
        GENERATE_MOVES_CASE(QUEEN_B)
        GENERATE_MOVES_CASE(PRINCESS_W)
        GENERATE_MOVES_CASE(PRINCESS_B)
        GENERATE_MOVES_CASE(PAWN_W)
        GENERATE_MOVES_CASE(BRAWN_W)
        GENERATE_MOVES_CASE(PAWN_B)
        GENERATE_MOVES_CASE(BRAWN_B)
        GENERATE_MOVES_CASE(PAWN_UW)
        GENERATE_MOVES_CASE(BRAWN_UW)
        GENERATE_MOVES_CASE(PAWN_UB)
        GENERATE_MOVES_CASE(BRAWN_UB)
        GENERATE_MOVES_CASE(KNIGHT_W)
        GENERATE_MOVES_CASE(KNIGHT_B)
        GENERATE_MOVES_CASE(UNICORN_W)
        GENERATE_MOVES_CASE(UNICORN_B)
        GENERATE_MOVES_CASE(DRAGON_W)
        GENERATE_MOVES_CASE(DRAGON_B)
#undef GENERATE_MOVES_CASE
    default:
        throw std::runtime_error("Unknown piece " + std::string({ (char)piece_name(p_piece) }) + (p_piece & 0x80 ? "*" : "") + "\n");
        break;
    }
	return mvbbs;
}

template<bool C>
std::map<vec4, bitboard_t> multiverse::gen_moves(vec4 p) const
{
    auto& all_moves = C ? all_moves_b : all_moves_w;
//    if(all_moves.contains(p))
//    {
//        std::cerr << "use cached value in gen_moves for " << p << std::endl;
//        return all_moves[p];
//    }
    std::shared_ptr<board> b_ptr = get_board(p.l(), p.t(), C);
    piece_t p_piece = b_ptr->get_piece(p.xy());
	if (b_ptr->umove() & pmask(p.xy()))
    {
        p_piece = static_cast<piece_t>(p_piece | 0x80);
    }
    std::map<vec4, bitboard_t> mvbbs;
    switch(p_piece)
    {
#define GENERATE_MOVES_CASE(PIECE) \
        case PIECE: \
            mvbbs = gen_superphysical_moves_impl<PIECE, C>(p); \
            mvbbs[p.tl()] = gen_physical_moves_impl<PIECE, C>(p); \
            break;

        GENERATE_MOVES_CASE(KING_W)
        GENERATE_MOVES_CASE(KING_B)
        GENERATE_MOVES_CASE(KING_UW)
        GENERATE_MOVES_CASE(KING_UB)
        GENERATE_MOVES_CASE(COMMON_KING_W)
        GENERATE_MOVES_CASE(COMMON_KING_B)
        GENERATE_MOVES_CASE(ROOK_W)
        GENERATE_MOVES_CASE(ROOK_B)
        GENERATE_MOVES_CASE(ROOK_UW)
        GENERATE_MOVES_CASE(ROOK_UB)
        GENERATE_MOVES_CASE(BISHOP_W)
        GENERATE_MOVES_CASE(BISHOP_B)
        GENERATE_MOVES_CASE(QUEEN_W)
        GENERATE_MOVES_CASE(QUEEN_B)
        GENERATE_MOVES_CASE(PRINCESS_W)
        GENERATE_MOVES_CASE(PRINCESS_B)
        GENERATE_MOVES_CASE(PAWN_W)
        GENERATE_MOVES_CASE(BRAWN_W)
        GENERATE_MOVES_CASE(PAWN_B)
        GENERATE_MOVES_CASE(BRAWN_B)
        GENERATE_MOVES_CASE(PAWN_UW)
        GENERATE_MOVES_CASE(BRAWN_UW)
        GENERATE_MOVES_CASE(PAWN_UB)
        GENERATE_MOVES_CASE(BRAWN_UB)
        GENERATE_MOVES_CASE(KNIGHT_W)
        GENERATE_MOVES_CASE(KNIGHT_B)
        GENERATE_MOVES_CASE(UNICORN_W)
        GENERATE_MOVES_CASE(UNICORN_B)
        GENERATE_MOVES_CASE(DRAGON_W)
        GENERATE_MOVES_CASE(DRAGON_B)
#undef GENERATE_MOVES_CASE
        default:
            throw std::runtime_error("Unknown piece " + std::string({(char)piece_name(p_piece)}) + (p_piece & 0x80 ? "*": "") + "\n");
            break;
    }
    all_moves[p] = mvbbs;
    return mvbbs;
}

std::vector<vec4> multiverse::gen_piece_move(vec4 p, int board_color) const
{
    std::map<vec4, bitboard_t> mvbbs = board_color ? gen_moves<true>(p) : gen_moves<false>(p);
    std::vector<vec4> result;
    for (const auto& [r, bb] : mvbbs)
    {
        for(int pos : marked_pos(bb))
        {
            vec4 q = vec4(pos, r) - p;
			//std::cerr << q << std::endl;
            result.push_back(q);
        }
    }
    return result;
}


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
std::map<vec4, bitboard_t> multiverse::gen_purely_sp_rook_moves(vec4 p0) const
{
    auto& sp_rook_moves = C ? sp_rook_moves_b : sp_rook_moves_w;
    if(sp_rook_moves.contains(p0.tl()))
    {
        return sp_rook_moves[p0.tl()];
    }
    std::map<vec4, bitboard_t> result;
    std::shared_ptr<board> b0_ptr = get_board(p0.l(), p0.t(), C);
    bitboard_t lrook = b0_ptr->lrook() & b0_ptr->friendly<C>();
    for(auto d : orthogonal_dtls)
    {
        bitboard_t remaining = lrook;
        for(vec4 p1 = p0 + d; remaining && inbound(p1, C); p1 = p1 + d)
        {
            std::shared_ptr<board> b1_ptr = get_board(p1.l(), p1.t(), C);
            remaining &= ~b1_ptr->friendly<C>();
            if(remaining)
            {
                result[p1.tl()] |= remaining;
                remaining &= ~b1_ptr->hostile<C>();
            }
        }
    }
    sp_rook_moves[p0.tl()] = result;
    return result;
}


template<bool C>
std::map<vec4, bitboard_t> multiverse::gen_purely_sp_bishop_moves(vec4 p0) const
{
    auto& sp_bishop_moves = C ? sp_bishop_moves_b : sp_bishop_moves_w;
    if(sp_bishop_moves.contains(p0.tl()))
    {
        return sp_bishop_moves[p0.tl()];
    }
    std::map<vec4, bitboard_t> result;
    std::shared_ptr<board> b0_ptr = get_board(p0.l(), p0.t(), C);
    bitboard_t lbishop = b0_ptr->lbishop() & b0_ptr->friendly<C>();
    for(auto d : diagonal_dtls)
    {
        bitboard_t remaining = lbishop;
        for(vec4 p1 = p0 + d; remaining && inbound(p1, C); p1 = p1 + d)
        {
            std::shared_ptr<board> b1_ptr = get_board(p1.l(), p1.t(), C);
            remaining &= ~b1_ptr->friendly<C>();
            if(remaining)
            {
                result[p1.tl()] |= remaining;
                remaining &= ~b1_ptr->hostile<C>();
            }
        }
    }
    sp_bishop_moves[p0.tl()] = result;
    return result;
}


template<bool C>
std::map<vec4, bitboard_t> multiverse::gen_purely_sp_knight_moves(vec4 p0) const
{
    auto& sp_knight_moves = C ? sp_knight_moves_b : sp_knight_moves_w;
    if(sp_knight_moves.contains(p0.tl()))
    {
        return sp_knight_moves[p0.tl()];
    }
    std::map<vec4, bitboard_t> result;
    std::shared_ptr<board> b0_ptr = get_board(p0.l(), p0.t(), C);
    bitboard_t lknight = b0_ptr->lknight() & b0_ptr->friendly<C>();
    const static std::vector<vec4> knight_pure_sp_delta = {vec4(0, 0, 2, 1), vec4(0, 0, 1, 2), vec4(0, 0, -2, 1), vec4(0, 0, 1, -2),
        vec4(0, 0, 2, -1), vec4(0, 0, -1, 2), vec4(0, 0, -2, -1), vec4(0, 0, -1, -2)};
    for(vec4 delta : knight_pure_sp_delta)
    {
        vec4 p1 = p0 + delta;
        if(inbound(p1, C))
        {
            std::shared_ptr<board> b1_ptr = get_board(p1.l(), p1.t(), C);
            bitboard_t remaining = lknight;
            remaining &= ~b1_ptr->friendly<C>();
            if(remaining)
            {
                result[p1.tl()] |= remaining;
            }
        }
    }
    sp_knight_moves[p0.tl()] = result;
    return result;
}


template<piece_t P, bool C>
bitboard_t multiverse::gen_physical_moves_impl(vec4 p) const
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
    else if constexpr (P == KING_UW || P == KING_UB)
    {
        a = king_attack(p.xy()) & ~friendly;
        bitboard_t urook = b_ptr->umove() & b_ptr->rook() & friendly;
        if(!b_ptr->is_under_attack(p.xy(), C))
        {
            for(vec4 d : {vec4(1,0,0,0), vec4(-1,0,0,0)})
            {
                int i = 0;
                for(vec4 q = p + d; !q.outbound(); q = q + d)
                {
                    bitboard_t w = pmask(q.xy());
                    if(i < 2 && b_ptr->is_under_attack(q.xy(), C))
                    {
                        break;
                    }
                    else if(w & urook)
                    {
                        a |= pmask((p + d + d).xy());
                        break;
                    }
                    else if(w & b_ptr->occupied())
                    {
                        break;
                    }
                    i++;
                }
            }
        }
    }
    else if constexpr (P == ROOK_W || P == ROOK_B || P == ROOK_UW || P == ROOK_UB)
    {
		a = rook_attack(p.xy(), b_ptr->occupied()) & ~friendly;
	}
    else if constexpr (P == BISHOP_W || P == BISHOP_B)
    {
		a = bishop_attack(p.xy(), b_ptr->occupied()) & ~friendly;
    }
    else if constexpr (P == QUEEN_W || P == QUEEN_B || P == PRINCESS_W || P == PRINCESS_B)
    {
        a = queen_attack(p.xy(), b_ptr->occupied()) & ~friendly;
	}
    else if constexpr (P == PAWN_W || P == BRAWN_W || P == PAWN_UW || P == BRAWN_UW)
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
    else if constexpr (P == PAWN_B || P == BRAWN_B || P == PAWN_UB || P == BRAWN_UB)
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
	else if constexpr (P == KNIGHT_W || P == KNIGHT_B)
	{
		a = knight_attack(p.xy()) & ~friendly;
	}
    else if constexpr (P == UNICORN_W || P == UNICORN_B || P == DRAGON_W || P == DRAGON_B)
    {
        a = 0;
    }
	else
	{
		std::cerr << "gen_physical_moves_impl:" << P << "not implemented" << std::endl;
	}
	return a;
}


template<bool C, multiverse::axesmode TL, multiverse::axesmode XY>
void multiverse::gen_compound_moves(vec4 p, std::map<vec4, bitboard_t>& result) const
{
    int pos = p.xy();
    bitboard_t occ, fri;
    bitboard_t copy_mask;
    
    constexpr auto deltas = (TL==multiverse::axesmode::ORTHOGONAL) ? orthogonal_dtls : (TL==multiverse::axesmode::DIAGONAL) ? diagonal_dtls : both_dtls;
    
    constexpr auto copy_mask_fn = (TL==multiverse::axesmode::ORTHOGONAL) ? rook_copy_mask : (TL==multiverse::axesmode::DIAGONAL) ? bishop_copy_mask : queen_copy_mask;

    for(vec4 d : deltas)
    {
        vec4 q = p;
        occ = fri = 0;
        for (int n = 1; n < 8; n++)
        {
            copy_mask = copy_mask_fn(pos, n);
            q = q + d;
            // if the corresponding board exists, copy the cone slice
            if(inbound(q, C))
            {
                std::shared_ptr<board> b_ptr = get_board(q.l(), q.t(), C);
                occ |= copy_mask & b_ptr->occupied();
                fri |= copy_mask & b_ptr->friendly<C>();
            }
            // otherwise, set the cone slice to a blocker of friendly pieces, which prevents the attacking move towards this non-existant board
            else
            {
                occ |= copy_mask;
                fri |= copy_mask;
                break;
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
        for (int n = 1; n < 8; n++)
        {
            copy_mask = copy_mask_fn(pos, n);
            q = q + d;
            bitboard_t c = loc & copy_mask;
            if(c) 
            {
                result[q.tl()] |= c;
            }
            else 
            {
                break;
            }
        }
    }
}

template<piece_t P, bool C>
std::map<vec4, bitboard_t>multiverse::gen_superphysical_moves_impl(vec4 p) const
{
    std::map<vec4, bitboard_t> result;
    int pos = p.xy();
    if constexpr (P == KING_W || P == KING_B || P == COMMON_KING_W || P == COMMON_KING_B || P == KING_UW || P == KING_UB)
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
                    result[q.tl()] |= bb;
                }
            }
        }
    }
    else if constexpr (P == ROOK_W || P == ROOK_B || P == ROOK_UW || P == ROOK_UB)
    {
        bitboard_t z = pmask(p.xy());
        for(auto [index, bb] : gen_purely_sp_rook_moves<C>(p))
        {
            bitboard_t bb1 = bb & z;
            if(bb1)
            {
                result[index.tl()] |= bb1;
            }
        }
    }
    else if constexpr (P == BISHOP_W || P == BISHOP_B)
    {
        bitboard_t z = pmask(p.xy());
        for(auto [index, bb] : gen_purely_sp_bishop_moves<C>(p))
        {
            bitboard_t bb1 = bb & z;
            if(bb1)
            {
                result[index.tl()] |= bb1;
            }
        }
        gen_compound_moves<C, multiverse::axesmode::ORTHOGONAL, multiverse::axesmode::ORTHOGONAL>(p, result);
    }
    else if constexpr (P == PRINCESS_W || P == PRINCESS_B)
    {
        result = gen_superphysical_moves_impl<ROOK_W,C>(p);
        result.merge(gen_superphysical_moves_impl<BISHOP_W,C>(p));
    }
    else if constexpr (P == QUEEN_W || P == QUEEN_B)
    {
        bitboard_t z = pmask(p.xy());
        for(auto [index, bb] : gen_purely_sp_rook_moves<C>(p))
        {
            bitboard_t bb1 = bb & z;
            if(bb1)
            {
                result[index.tl()] |= bb1;
            }
        }
        for(auto [index, bb] : gen_purely_sp_bishop_moves<C>(p))
        {
            bitboard_t bb1 = bb & z;
            if(bb1)
            {
                result[index.tl()] |= bb1;
            }
        }
        gen_compound_moves<C, multiverse::axesmode::BOTH, multiverse::axesmode::BOTH>(p, result);
    }
    else if constexpr (P == PAWN_W || P == BRAWN_W || P == PAWN_UW || P == BRAWN_UW)
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
                    result[q.tl()] |= bb;
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
                    if(inbound(r,C))
                    {
                        std::shared_ptr<board> b1_ptr = get_board(r.l(), r.t(), C);
                        bitboard_t bc = z & ~b_ptr->occupied();
                        if(bc)
                        {
                            result[r.tl()] |= bc;
                        }
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
                result[q.tl()] |= bb;
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
                    result[s.tl()] |= bd;
                }
            }
        }
    }
    else if constexpr (P == PAWN_B || P == BRAWN_B || P == PAWN_UB || P == BRAWN_UB)
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
                    result[q.tl()] |= bb;
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
                    if(inbound(r,C))
                    {
                        std::shared_ptr<board> b1_ptr = get_board(r.l(), r.t(), C);
                        bitboard_t bc = z & ~b_ptr->occupied();
                        if(bc)
                        {
                            result[r.tl()] |= bc;
                        }
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
                result[q.tl()] |= bb;
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
                    result[s.tl()] |= bd;
                }
            }
        }
    }
    else if constexpr (P == KNIGHT_W || P == KNIGHT_B)
    {
        for(auto [index, bb] : gen_purely_sp_knight_moves<C>(p))
        {
            bitboard_t bb1 = bb & pmask(pos);
            if(bb1)
            {
                result[index.tl()] |= bb1;
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
                    result[q.tl()] |= bb;
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
                    result[q.tl()] |= bb;
                }
            }
        }
    }
    else if constexpr (P == UNICORN_W || P == UNICORN_B)
    {
        gen_compound_moves<C, multiverse::axesmode::ORTHOGONAL, multiverse::axesmode::DIAGONAL>(p, result);
        gen_compound_moves<C, multiverse::axesmode::DIAGONAL, multiverse::axesmode::ORTHOGONAL>(p, result);
    }
    else if constexpr (P == DRAGON_W || P == DRAGON_B)
    {
        gen_compound_moves<C, multiverse::axesmode::DIAGONAL, multiverse::axesmode::DIAGONAL>(p, result);
    }
    else
    {
        std::cerr << "gen_superphysical_moves_impl:" << P << "not implemented" << std::endl;
    }
    return result;
}


// Explicit instantiation of the template for specific types
template bitboard_t multiverse::gen_physical_moves_impl<KING_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KING_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KING_UW, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KING_UB, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<COMMON_KING_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<COMMON_KING_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<ROOK_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<ROOK_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<ROOK_UW, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<ROOK_UB, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<BISHOP_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<BISHOP_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<QUEEN_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<QUEEN_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PRINCESS_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PRINCESS_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PAWN_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PAWN_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PAWN_UW, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PAWN_UB, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KNIGHT_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KNIGHT_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<UNICORN_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<UNICORN_B, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<DRAGON_W, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<DRAGON_B, true>(vec4 p) const;

template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KING_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KING_B, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KING_UW, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KING_UB, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<COMMON_KING_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<COMMON_KING_B, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<ROOK_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<ROOK_B, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<ROOK_UW, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<ROOK_UB, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<BISHOP_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<BISHOP_B, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<QUEEN_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<QUEEN_B, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PRINCESS_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PRINCESS_B, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PAWN_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PAWN_B, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PAWN_UW, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PAWN_UB, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KNIGHT_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KNIGHT_B, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<UNICORN_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<UNICORN_B, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<DRAGON_W, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<DRAGON_B, true>(vec4 p) const;

template std::map<vec4, bitboard_t> multiverse::gen_purely_sp_rook_moves<false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_purely_sp_rook_moves<true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_purely_sp_bishop_moves<false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_purely_sp_bishop_moves<true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_purely_sp_knight_moves<false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_purely_sp_knight_moves<true>(vec4 p) const;

template void multiverse::gen_compound_moves<false, multiverse::axesmode::ORTHOGONAL, multiverse::axesmode::ORTHOGONAL>(vec4 p, std::map<vec4, bitboard_t>& result) const;
template void multiverse::gen_compound_moves<true, multiverse::axesmode::ORTHOGONAL, multiverse::axesmode::ORTHOGONAL>(vec4 p, std::map<vec4, bitboard_t>& result) const;
template void multiverse::gen_compound_moves<false, multiverse::axesmode::DIAGONAL, multiverse::axesmode::DIAGONAL>(vec4 p, std::map<vec4, bitboard_t>& result) const;
template void multiverse::gen_compound_moves<true, multiverse::axesmode::DIAGONAL, multiverse::axesmode::DIAGONAL>(vec4 p, std::map<vec4, bitboard_t>& result) const;
template void multiverse::gen_compound_moves<false, multiverse::axesmode::BOTH, multiverse::axesmode::BOTH>(vec4 p, std::map<vec4, bitboard_t>& result) const;
template void multiverse::gen_compound_moves<true, multiverse::axesmode::BOTH, multiverse::axesmode::BOTH>(vec4 p, std::map<vec4, bitboard_t>& result) const;

template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves<true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves<false>(vec4 p) const;

template std::map<vec4, bitboard_t> multiverse::gen_moves<true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_moves<false>(vec4 p) const;

// perhaps these template specializations will never get called
// but I decide to include them for completeness & prevent disasterious compiler errors
template bitboard_t multiverse::gen_physical_moves_impl<KING_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KING_B, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KING_UW, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KING_UB, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<COMMON_KING_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<COMMON_KING_B, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<ROOK_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<ROOK_B, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<ROOK_UW, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<ROOK_UB, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<BISHOP_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<BISHOP_B, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<QUEEN_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<QUEEN_B, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PRINCESS_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PRINCESS_B, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PAWN_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PAWN_B, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PAWN_UW, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<PAWN_UB, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KNIGHT_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<KNIGHT_B, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<UNICORN_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<UNICORN_B, false>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<DRAGON_W, true>(vec4 p) const;
template bitboard_t multiverse::gen_physical_moves_impl<DRAGON_B, false>(vec4 p) const;

template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KING_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KING_B, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KING_UW, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KING_UB, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<COMMON_KING_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<COMMON_KING_B, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<ROOK_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<ROOK_B, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<ROOK_UW, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<ROOK_UB, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<BISHOP_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<BISHOP_B, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<QUEEN_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<QUEEN_B, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PRINCESS_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PRINCESS_B, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PAWN_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PAWN_B, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PAWN_UW, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<PAWN_UB, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KNIGHT_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<KNIGHT_B, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<UNICORN_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<UNICORN_B, false>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<DRAGON_W, true>(vec4 p) const;
template std::map<vec4, bitboard_t> multiverse::gen_superphysical_moves_impl<DRAGON_B, false>(vec4 p) const;
