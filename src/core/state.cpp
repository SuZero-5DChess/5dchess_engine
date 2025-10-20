#include "state.h"
#include <algorithm>
#include <cassert>
#include "utils.h"

state::state(multiverse &mtv) : m(mtv.clone())//, match_status(match_status_t::PLAYING)
{
    std::tie(present, player) = m->get_present();
}

int state::new_line() const
{
    auto [l_min, l_max] = m->get_lines_range();
    if(player == 0)
        return l_max + 1;
    else
        return l_min - 1;
}

std::optional<state> state::can_submit() const
{
    state new_state = *this;
    bool flag = new_state.submit<false>();
    if(flag)
    {
        return std::optional<state>(new_state);
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<state> state::can_apply(full_move fm) const
{
    state new_state = *this;
    bool flag = new_state.apply_move<false>(fm);
    if(flag)
    {
        return std::optional<state>(new_state);
    }
    else
    {
        return std::nullopt;
    }
}

template<bool UNSAFE>
bool state::apply_move(full_move fm)
{
//    if constexpr (!UNSAFE)
//    {
//        if(match_status != match_status_t::PLAYING)
//        {
//            std::cerr << "In apply_move<" << UNSAFE << ">(" << fm << "):\n";
//            std::cerr << "Match has already ended with outcome " << match_status << "\n";
//            throw std::runtime_error("Attempt to move finalized game.\n");
//        }
//    }
    vec4 p = fm.from;
    vec4 q = fm.to;
    vec4 d = q - p;
    if constexpr (!UNSAFE)
    {
        auto mvs = player ? m->gen_moves<true>(p) : m->gen_moves<false>(p);
        //auto it = mvbbs.find(q.tl());
        const auto &res = mvs.find([&q](const auto &pair){
            const auto &[tl, bb] = pair;
            return tl == q.tl();
        });
        // is it a pseudolegal move?
        if(res)
        {
            bitboard_t bb = res.value().second;
            if(!(pmask(q.xy()) & bb))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    
    // physical move, no time travel
    if(d.l() == 0 && d.t() == 0)
    {
        const std::shared_ptr<board>& b_ptr = m->get_board(p.l(), p.t(), player);
        piece_t pic = b_ptr->get_piece(p.xy());
        // en passant
        if(to_white(pic) == PAWN_W && d.x()!=0 && b_ptr->get_piece(q.xy()) == NO_PIECE)
        {
            //std::cout << " ... en passant";
            m->append_board(p.l(),
                            b_ptr->replace_piece(ppos(q.x(),p.y()), NO_PIECE)
                            ->move_piece(p.xy(), q.xy()));
        }
        // TODO: promotion
        // castling
        else if(to_white(pic) == KING_W && abs(d.x()) > 1)
        {
            //std::cout << " ... castling";
            int rook_x1 = d.x() < 0 ? 0 : 7;
            int rook_x2 = q.x() + (d.x() < 0 ? 1 : -1);
            m->append_board(p.l(),b_ptr
                            ->move_piece(ppos(rook_x1, p.y()), ppos(rook_x2,q.y()))
                            ->move_piece(p.xy(), q.xy()));
        }
        // normal move
        else
        {
            //std::cout << " ... normal move/capture";
            m->append_board(p.l(), b_ptr->move_piece(p.xy(), q.xy()));
        }
    }
    // non-branching superphysical move
    //else if(multiverse::tc_to_v(q.t(), player) == m->timeline_end[multiverse::l_to_u(q.l())])
    else if (std::make_pair(q.t(), player) == m->get_timeline_end(q.l()))
    {
        //std::cout << " ... nonbranching move";
        const std::shared_ptr<board>& b_ptr = m->get_board(p.l(), p.t(), player);
        const piece_t& pic = static_cast<piece_t>(piece_name(b_ptr->get_piece(p.xy())));
        m->append_board(p.l(), b_ptr->replace_piece(p.xy(), NO_PIECE));
        const std::shared_ptr<board>& c_ptr = m->get_board(q.l(), q.t(), player);
        m->append_board(q.l(), c_ptr->replace_piece(q.xy(), pic));
    }
    //branching move
    else
    {
        //std::cout << " ... branching move";
        const std::shared_ptr<board>& b_ptr = m->get_board(p.l(), p.t(), player);
        const piece_t& pic = static_cast<piece_t>(piece_name(b_ptr->get_piece(p.xy())));
        m->append_board(p.l(), b_ptr->replace_piece(p.xy(), NO_PIECE));
        const std::shared_ptr<board>& x_ptr = m->get_board(q.l(), q.t(), player);
        int v = (q.t() << 1 | player) + 1;
        int t = v >> 1;
        int c = v & 1;
        m->insert_board(new_line(), t, c, x_ptr->replace_piece(q.xy(), pic));
        auto [new_present, _] = m->get_present();
        if(new_present < present)
        {
            // if a historical board is activated by this travel, go back
            present = new_present;
        }
    }
    return true;
}

template <bool UNSAFE>
bool state::submit()
{
    auto [present_t, present_c] = m->get_present();
    if constexpr (!UNSAFE)
    {
        if(player == present_c)
        {
            return false;
        }
    }
    present = present_t;
    player  = present_c;
    return true;
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> state::get_timeline_status() const
{
    return get_timeline_status(present, player);
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> state::get_timeline_status(int present_t, int present_c) const
{
    auto [l_min, l_max] = m->get_lines_range();
    auto [active_min, active_max] = m->get_active_range();
    auto present_v = std::make_pair(present_t, present_c);
    std::vector<int> mandatory_timelines, optional_timelines, unplayable_timelines;
    for(int l = l_min; l <= l_max; l++)
    {
        auto v = m->get_timeline_end(l);
        //int v = m->timeline_end[multiverse::l_to_u(l)];
        if (active_min <= l && active_max >= l && v == present_v)
        {
            mandatory_timelines.push_back(l);
        }
        else
        {
            auto [t, c] = v;
            if(present_c==c)
            {
                optional_timelines.push_back(l);
            }
            else
            {
                unplayable_timelines.push_back(l);
            }
        }
    }
    return std::make_tuple(mandatory_timelines, optional_timelines, unplayable_timelines);
}


/**********
 *********
 ********
 *******
 ******
 *****
 ****
 ***
 **
 */

bool state::find_check() const
{
    auto [mandatory_timelines, optional_timelines, unplayable_timelines] = get_timeline_status();
    std::list<int> lines;
    std::copy(mandatory_timelines.begin(), mandatory_timelines.end(), std::back_inserter(lines));
    std::copy(optional_timelines.begin(), optional_timelines.end(), std::back_inserter(lines));
    // find checks on the opponents timelines
	if (player == 0)
	{
		return find_check_impl<true>(lines);
	}
	else
	{
		return find_check_impl<false>(lines);
	}
}

template<bool C>
bool state::find_check_impl(const std::list<int>& lines) const
{
    std::set<vec4> checking;
//    std::cerr << "\nIn find_check_impl<" << C << ">(";
//    for(int l : lines)
//    {
//        std::cerr << l << ",";
//    }
//    std::cerr << "\b)\n";
    for (int l : lines)
    {
        // take the active board
        auto [t, c] = m->get_timeline_end(l);
//        std::cerr << c << " " << C << "\n";
//        assert(c == C);
        std::shared_ptr<board> b_ptr = m->get_board(l, t, C);
        bitboard_t b_pieces = b_ptr->friendly<C>() & ~b_ptr->wall();
        // find checks for the physical moves
        for (int pos : marked_pos(b_ptr->hostile<C>()& b_ptr->royal()))
        {
            if (b_ptr->is_under_attack(pos, !C))
            {
				//std::cerr << "The royal piece is under attack on " << vec4(pos, vec4(0, 0, t, l)) << std::endl;
                return true;
            }
        }
		// find checks for the superphysical moves
        // for each friendly piece on this board
        for (int src_pos : marked_pos(b_pieces))
        {
            vec4 p = vec4(src_pos, vec4(0,0,t,l));
            // generate the aviliable superphysical moves
            auto moves = m->gen_superphysical_moves<C>(p);
//            std::cerr << "The allowed moves are: ";
//            for(vec4 d : m->gen_piece_move(p, C))
//            {
//                std::cerr << move5d::move(p, d) << " ";
//            }
//            std::cerr << std::endl;
            // for each destination board and bit location
//            std::cerr << "for piece " << m->get_piece(p, C) << " on " << p << ":\n";
            for (const auto& [q0, bb] : moves)
            {
//                std::cerr << "q0 = " << q0 << std::endl;
                std::shared_ptr<board> b1_ptr = m->get_board(q0.l(), q0.t(), C);
                if (bb)
                {
                    // if the destination square is royal, this is a check
                    if (bb & b1_ptr->royal())
                    {
						//std::cerr << "Found a check on " << q0 << std::endl;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


std::vector<full_move> state::find_all_checks() const
{
    auto [mandatory_timelines, optional_timelines, unplayable_timelines] = get_timeline_status();
    std::list<int> lines;
    std::copy(mandatory_timelines.begin(), mandatory_timelines.end(), std::back_inserter(lines));
    std::copy(optional_timelines.begin(), optional_timelines.end(), std::back_inserter(lines));
    // find checks on the opponents timelines
    if (player == 0)
    {
        return find_all_checks_impl<true>(lines);
    }
    else
    {
        return find_all_checks_impl<false>(lines);
    }
}

template<bool C>
std::vector<full_move> state::find_all_checks_impl(const std::list<int>& lines) const
{
    std::vector<full_move> checking;
    for (int l : lines)
    {
        // take the active board
        auto [t, c] = m->get_timeline_end(l);
//        assert(c == C);
        std::shared_ptr<board> b_ptr = m->get_board(l, t, C);
        bitboard_t b_pieces = b_ptr->friendly<C>() & ~b_ptr->wall();
        // for each friendly piece on this board
        for (int src_pos : marked_pos(b_pieces))
        {
            vec4 p = vec4(src_pos, vec4(0,0,t,l));
            // generate the aviliable moves
            auto moves = m->gen_moves<C>(p);
            // for each destination board and bit location
            for (const auto& [q0, bb] : moves)
            {
                std::shared_ptr<board> b1_ptr = m->get_board(q0.l(), q0.t(), C);
                if (bb)
                {
                    // if the destination square is royal, this is a check
                    bitboard_t c_pieces = bb & b1_ptr->royal();
                    if (c_pieces)
                    {
                        for(int dst_pos : marked_pos(c_pieces))
                        {
                            vec4 q = vec4(dst_pos, q0);
                            checking.push_back({p, q});
                        }
                    }
                }
            }
        }
    }
    return checking;
}


std::map<vec4, bitboard_t> state::gen_movable_pieces() const
{
    auto [mandatory_timelines, optional_timelines, unplayable_timelines] = get_timeline_status(present, player);
    auto lines = concat_vectors(mandatory_timelines, optional_timelines);
    if (player == 0)
    {
        return gen_movable_pieces_impl<false>(lines);
    }
    else
    {
        return gen_movable_pieces_impl<true>(lines);
    }
}

std::map<vec4, bitboard_t> state::get_movable_pieces(std::vector<int> lines) const
{
    if (player == 0)
    {
        return gen_movable_pieces_impl<false>(lines);
    }
    else
    {
        return gen_movable_pieces_impl<true>(lines);
    }
}

template <bool C>
std::map<vec4, bitboard_t> state::gen_movable_pieces_impl(const std::vector<int> &lines) const
{
    std::map<vec4, bitboard_t> movable;
    for (int l : lines)
    {
        // take the active board
        auto [t, c] = get_timeline_end(l);
        const vec4 p0 = vec4(0,0,t,l);
//        assert(c == C);
        std::shared_ptr<board> b_ptr = m->get_board(l, t, C);
        bitboard_t b_pieces = b_ptr->friendly<C>() & ~b_ptr->wall();
        // for each friendly piece on this board
        for (int src_pos : marked_pos(b_pieces))
        {
            vec4 p = vec4(src_pos, p0);
            // generate the aviliable moves
            auto moves = m->gen_moves<C>(p);
            // for each destination board and bit location
            bool still = true;
            for (const auto& [q0, bb] : moves)
            {
                if (bb)
                {
                    still = false;
                    break;
                }
            }
            if(still)
            {
                // if this is a still pos, remove it mask
                b_pieces &= ~pmask(src_pos);
            }
        }
        if(b_pieces)
        {
            movable[p0] = b_pieces;
        }
    }
    return movable;
}

std::pair<int, int> state::apparent_present() const
{
    return m->get_present();
}

std::pair<int, int> state::get_lines_range() const
{
    return m->get_lines_range();
}

std::pair<int, int> state::get_active_range() const
{
    return m->get_active_range();
}

std::pair<int, int> state::get_timeline_start(int l) const
{
    return m->get_timeline_start(l);
}

std::pair<int, int> state::get_timeline_end(int l) const
{
    return m->get_timeline_end(l);
}

piece_t state::get_piece(vec4 p, int color) const
{
    return m->get_piece(p, color);
}

std::shared_ptr<board> state::get_board(int l, int t, int c) const
{
    return m->get_board(l, t, c);
}

std::vector<std::tuple<int, int, int, std::string>> state::get_boards() const
{
    return std::vector<std::tuple<int, int, int, std::string>>();
}

generator<vec4> state::gen_piece_move(vec4 p) const
{
    return m->gen_piece_move(p, player);
}

std::string state::to_string() const
{
    std::ostringstream ss;
    ss << "State(present=" << present << ", player=" << player << "):\n";
    return ss.str() + m->to_string();
}

//std::ostream& operator<<(std::ostream& os, const match_status_t& status)
//{
//    switch (status)
//    {
//        case match_status_t::PLAYING:
//            os << "PLAYING";
//            break;
//        case match_status_t::WHITE_WINS:
//            os << "WHITE_WINS";
//            break;
//        case match_status_t::BLACK_WINS:
//            os << "BLACK_WINS";
//            break;
//        case match_status_t::STALEMATE:
//            os << "STALEMATE";
//            break;
//    }
//    return os;
//}

template bool state::apply_move<false>(full_move);
template bool state::apply_move<true>(full_move);
template bool state::submit<false>();
template bool state::submit<true>();

template bool state::find_check_impl<false>(const std::list<int>&) const;
template bool state::find_check_impl<true>(const std::list<int>&) const;

template std::map<vec4, bitboard_t> state::gen_movable_pieces_impl<false>(const std::vector<int>&) const;
template std::map<vec4, bitboard_t> state::gen_movable_pieces_impl<true>(const std::vector<int>&) const;

template std::vector<full_move> state::find_all_checks_impl<false>(const std::list<int>&) const;
template std::vector<full_move> state::find_all_checks_impl<true>(const std::list<int>&) const;
