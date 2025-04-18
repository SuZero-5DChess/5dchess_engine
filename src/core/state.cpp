#include "state.h"
#include <algorithm>
#include <cassert>
#include "utils.h"

state::state(multiverse mtv) : m(mtv), match_status(match_status_t::PLAYING)
{
    std::tie(present, player) = m.get_present();
}

int state::new_line() const
{
    if(player == 0)
        return m.l_max + 1;
    else
        return m.l_min - 1;
}

// state::state(const state& other)
//     : m(other.m),
//       present(other.present),
//       player(other.player),
//       match_status(other.match_status),
//       critical_coords{} // do not copy cache
// {}

// state& state::operator=(const state& other)
// {
//     if (this != &other) {
//         m = other.m;
//         present = other.present;
//         player = other.player;
//         match_status = other.match_status;
//         critical_coords.clear(); // reset cache
//     }
//     return *this;
// }

// void state::clear_cache()
// {
//     critical_coords.clear();
// }

bool state::can_submit() const
{
    auto [present_t, present_c] = m.get_present();
//    std::cout << present_t << " " << present_c << std::endl;
//    std::cout << present << " " << player << std::endl;
//    std::cout << (player != present_c) << (present != present_t);
    return player != present_c || present != present_t;
}

template<bool UNSAFE>
bool state::apply_move(full_move fm)
{
    if constexpr (!UNSAFE)
    {
        if(match_status != match_status_t::PLAYING)
        {
            std::cerr << "In apply_move<" << UNSAFE << ">(" << fm << "):\n";
            std::cerr << "Match has already ended with outcome " << match_status << "\n";
            throw std::runtime_error("Attempt to move finalized game.\n");
        }
    }
    bool flag;
//    std::cerr << "apply_move<" << UNSAFE << ">(" << fm << ")\n";
    std::visit(overloads {
        [&](std::monostate)
        {
            if constexpr (!UNSAFE)
            {
                // can present be shifted to the opponent?
                if(!can_submit())
                {
                    flag = false;
                    return;
                }
            }
            auto [present_t, present_c] = m.get_present();
            present = present_t;
            player  = present_c;
            flag = true;
        },
        [&](std::tuple<vec4, vec4> data)
        {
            const auto& [p, d] = data;
            vec4 q = p+d;
            if constexpr (!UNSAFE)
            {
                vec4 q = p+d;
                std::map<vec4, bitboard_t> mvbbs = player ? m.gen_moves<true>(p) : m.gen_moves<false>(p);
                auto it = mvbbs.find(q.tl());
                // is it a pseudolegal move?
                if(it != mvbbs.end())
                {
                    bitboard_t bb = mvbbs[q.tl()];
                    if(!(pmask(q.xy()) & bb))
                    {
                        flag = false;
                        return;
                    }
                }
                else
                {
                    flag = false;
                    return;
                }
            }
            
            // physical move, no time travel
            if(d.l() == 0 && d.t() == 0)
            {
                const std::shared_ptr<board>& b_ptr = m.get_board(p.l(), p.t(), player);
                piece_t pic = b_ptr->get_piece(p.xy());
                // en passant
                if(to_white(pic) == PAWN_W && d.x()!=0 && b_ptr->get_piece(q.xy()) == NO_PIECE)
                {
                    //std::cout << " ... en passant";
                    m.append_board(p.l(),
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
                    m.append_board(p.l(),b_ptr
                                   ->move_piece(ppos(rook_x1, p.y()), ppos(rook_x2,q.y()))
                                   ->move_piece(p.xy(), q.xy()));
                }
                // normal move
                else
                {
                    //std::cout << " ... normal move/capture";
                    m.append_board(p.l(), b_ptr->move_piece(p.xy(), q.xy()));
                }
            }
            // non-branching superphysical move
            else if(multiverse::tc_to_v(q.t(), player) == m.timeline_end[multiverse::l_to_u(q.l())])
            {
                //std::cout << " ... nonbranching move";
                const std::shared_ptr<board>& b_ptr = m.get_board(p.l(), p.t(), player);
                const piece_t& pic = static_cast<piece_t>(piece_name(b_ptr->get_piece(p.xy())));
                m.append_board(p.l(), b_ptr->replace_piece(p.xy(), NO_PIECE));
                const std::shared_ptr<board>& c_ptr = m.get_board(q.l(), q.t(), player);
                m.append_board(q.l(), c_ptr->replace_piece(q.xy(), pic));
            }
            //branching move
            else
            {
                //std::cout << " ... branching move";
                const std::shared_ptr<board>& b_ptr = m.get_board(p.l(), p.t(), player);
                const piece_t& pic = static_cast<piece_t>(piece_name(b_ptr->get_piece(p.xy())));
                m.append_board(p.l(), b_ptr->replace_piece(p.xy(), NO_PIECE));
                const std::shared_ptr<board>& x_ptr = m.get_board(q.l(), q.t(), player);
                auto [t, c] = multiverse::v_to_tc(multiverse::tc_to_v(q.t(), player)+1);
                m.insert_board(new_line(), t, c, x_ptr->replace_piece(q.xy(), pic));
                auto [new_present, _] = m.get_present();
                if(new_present < present)
                {
                    // if a historical board is activated by this travel, go back
                    present = new_present;
                }
            }
            // clear the cache for find_check() method because state has modified
            //clear_cache();
            // is this a legal move? (check detection here)
            if constexpr (!UNSAFE)
            {
                bool is_check = find_check();
                flag = !is_check;
				if (!flag)
				{
					std::cerr << "In apply_move<" << UNSAFE << ">(" << fm << "):\n";
					std::cerr << "This move creates a check!\n";
				}
            }
            else
            {
                flag = true;
            }
        }
    }, fm.data);
    return flag;
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> state::get_timeline_status() const
{
    return get_timeline_status(present, player);
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> state::get_timeline_status(int present_t, int present_c) const
{
    std::vector<int> mandatory_timelines, optional_timelines, unplayable_timelines;
    int present_v = multiverse::tc_to_v(present_t, present_c);
    for(int l = m.l_min; l <= m.l_max; l++)
    {
        int v = m.timeline_end[multiverse::l_to_u(l)];
        if (std::max(m.l_min, -m.number_activated()) <= l
            && std::min(m.l_max, m.number_activated()) >= l
            && v == present_v)
        {
            mandatory_timelines.push_back(l);
        }
        else
        {
            auto [t, c] = multiverse::v_to_tc(v);
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

bool state::find_check() const
{
    std::list<int> lines;
    int next_v = multiverse::tc_to_v(present, player) + 1;
    for(int l = m.l_min; l <= m.l_max; l++)
    {
        int v = m.timeline_end[multiverse::l_to_u(l)];
        if (std::max(m.l_min, -m.number_activated()) <= l
            && std::min(m.l_max, m.number_activated()) >= l
            && v == next_v)
        {
            //if this board is on a mandatory line, then it is more likely to create a check
            lines.push_front(l);
        }
        else
        {
            auto [t, c] = multiverse::v_to_tc(v);
            if(player!=c)
            {
                //if this board is on an optional line, it is still possible to create a check
                lines.push_back(l);
            }
        }
    }
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
    std::cerr << "\nIn find_check_impl<" << C << ">(";
    for(int l : lines)
    {
        std::cerr << l << ",";
    }
    std::cerr << "\b)\n";
    for (int l : lines)
    {
        // take the active board
        int v = m.timeline_end[multiverse::l_to_u(l)];
        auto [t, c] = multiverse::v_to_tc(v);
//        std::cerr << c << " " << C << "\n";
        assert(c == C);
        std::shared_ptr<board> b_ptr = m.get_board(l, t, C);
        bitboard_t b_pieces = b_ptr->friendly<C>();
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
            std::map<vec4, bitboard_t> moves = m.gen_superphysical_moves<C>(p);
//            std::cerr << "The allowed moves are: ";
//            for(vec4 d : m.gen_piece_move(p, C))
//            {
//                std::cerr << full_move::move(p, d) << " ";
//            }
//            std::cerr << std::endl;
            // for each destination board and bit location
//            std::cerr << "for piece " << m.get_piece(p, C) << " on " << p << ":\n";
            for (const auto& [q0, bb] : moves)
            {
//                std::cerr << "q0 = " << q0 << std::endl;
                std::shared_ptr<board> b1_ptr = m.get_board(q0.l(), q0.t(), C);
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


std::vector<std::pair<vec4,vec4>> state::find_all_checks() const
{
    std::list<int> lines;
    int next_v = multiverse::tc_to_v(present, player) + 1;
    for(int l = m.l_min; l <= m.l_max; l++)
    {
        int v = m.timeline_end[multiverse::l_to_u(l)];
        if (std::max(m.l_min, -m.number_activated()) <= l
            && std::min(m.l_max, m.number_activated()) >= l
            && v == next_v)
        {
            //if this board is on a mandatory line, then it is more likely to create a check
            lines.push_front(l);
        }
        else
        {
            auto [t, c] = multiverse::v_to_tc(v);
            if(player!=c)
            {
                //if this board is on an optional line, it is still possible to create a check
                lines.push_back(l);
            }
        }
    }
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
std::vector<std::pair<vec4,vec4>> state::find_all_checks_impl(const std::list<int>& lines) const
{
    std::vector<std::pair<vec4,vec4>> checking;
    for (int l : lines)
    {
        // take the active board
        int v = m.timeline_end[multiverse::l_to_u(l)];
        auto [t, c] = multiverse::v_to_tc(v);
        assert(c == C);
        std::shared_ptr<board> b_ptr = m.get_board(l, t, C);
        bitboard_t b_pieces = b_ptr->friendly<C>();
        // for each friendly piece on this board
        for (int src_pos : marked_pos(b_pieces))
        {
            vec4 p = vec4(src_pos, vec4(0,0,t,l));
            // generate the aviliable moves
            std::map<vec4, bitboard_t> moves = m.gen_moves<C>(p);
            // for each destination board and bit location
            for (const auto& [q0, bb] : moves)
            {
                std::shared_ptr<board> b1_ptr = m.get_board(q0.l(), q0.t(), C);
                if (bb)
                {
                    // if the destination square is royal, this is a check
                    bitboard_t c_pieces = bb & b1_ptr->royal();
                    if (c_pieces)
                    {
                        for(int dst_pos : marked_pos(c_pieces))
                        {
                            vec4 q = vec4(dst_pos, q0);
                            checking.push_back(std::make_pair(p, q));
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

template <bool C>
std::map<vec4, bitboard_t> state::gen_movable_pieces_impl(const std::vector<int> &lines) const
{
    std::map<vec4, bitboard_t> movable;
    for (int l : lines)
    {
        // take the active board
        int v = m.timeline_end[multiverse::l_to_u(l)];
        auto [t, c] = multiverse::v_to_tc(v);
        const vec4 p0 = vec4(0,0,t,l);
        assert(c == C);
        std::shared_ptr<board> b_ptr = m.get_board(l, t, C);
        bitboard_t b_pieces = b_ptr->friendly<C>();
        // for each friendly piece on this board
        for (int src_pos : marked_pos(b_pieces))
        {
            vec4 p = vec4(src_pos, p0);
            // generate the aviliable moves
            std::map<vec4, bitboard_t> moves = m.gen_moves<C>(p);
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

std::ostream& operator<<(std::ostream& os, const match_status_t& status)
{
    switch (status)
    {
        case match_status_t::PLAYING:
            os << "PLAYING";
            break;
        case match_status_t::WHITE_WINS:
            os << "WHITE_WINS";
            break;
        case match_status_t::BLACK_WINS:
            os << "BLACK_WINS";
            break;
        case match_status_t::STALEMATE:
            os << "STALEMATE";
            break;
    }
    return os;
}

template bool state::apply_move<false>(full_move);
template bool state::apply_move<true>(full_move);

template bool state::find_check_impl<false>(const std::list<int>&) const;
template bool state::find_check_impl<true>(const std::list<int>&) const;

template std::map<vec4, bitboard_t> state::gen_movable_pieces_impl<false>(const std::vector<int>&) const;
template std::map<vec4, bitboard_t> state::gen_movable_pieces_impl<true>(const std::vector<int>&) const;

template std::vector<std::pair<vec4,vec4>> state::find_all_checks_impl<false>(const std::list<int>&) const;
template std::vector<std::pair<vec4,vec4>> state::find_all_checks_impl<true>(const std::list<int>&) const;
