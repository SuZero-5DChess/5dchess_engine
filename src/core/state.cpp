#include "state.h"
#include <algorithm>
#include <cassert>
#include "utils.h"

std::pair<int, int> next_tc(int t, int c)
{
    int v = (t << 1 | c) + 1;
    return std::make_pair(v >> 1, v & 1);
}

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
        auto [t, c] = next_tc(q.t(), player);
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

generator<full_move> state::find_checks() const
{
    auto [t, c] = next_tc(present, player);
    auto [lines, optional_timelines, unplayable_timelines] = get_timeline_status(t, c);
    append_vectors(lines, optional_timelines);
//    print_range("playable: ", lines);
//    print_range("unplayable: ", unplayable_timelines);
    // find checks on the opponents timelines
    if (player == 0)
    {
        return find_checks_impl<true>(lines);
    }
    else
    {
        return find_checks_impl<false>(lines);
    }
}

template<bool C>
generator<full_move> state::find_checks_impl(std::vector<int> lines) const
{
//    print_range(__PRETTY_FUNCTION__, lines);
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
                            co_yield full_move(p, q);
                        }
                    }
                }
            }
        }
    }
}


std::vector<vec4> state::gen_movable_pieces() const
{
    auto [mandatory_timelines, optional_timelines, unplayable_timelines] = get_timeline_status(present, player);
    auto lines = concat_vectors(mandatory_timelines, optional_timelines);
    return get_movable_pieces(lines);
}

std::vector<vec4> state::get_movable_pieces(std::vector<int> lines) const
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
std::vector<vec4> state::gen_movable_pieces_impl(std::vector<int> lines) const
{
    std::vector<vec4> result;
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
            if(auto info = moves.first())
            {
                result.push_back(p);
            }
        }
    }
    return result;
}

std::pair<int, int> state::get_present() const
{
    return std::make_pair(present, player);
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
    return m->get_boards();
}

generator<vec4> state::gen_piece_move(vec4 p) const
{
    return m->gen_piece_move(p, player);
}

generator<vec4> state::gen_piece_move(vec4 p, int c) const
{
    return m->gen_piece_move(p, c);
}

std::string state::to_string() const
{
    std::ostringstream ss;
    ss << "State(present=" << present << ", player=" << player << "):\n";
    return ss.str() + m->to_string();
}

template bool state::apply_move<false>(full_move);
template bool state::apply_move<true>(full_move);
template bool state::submit<false>();
template bool state::submit<true>();

template generator<full_move> state::find_checks_impl<false>(std::vector<int>) const;
template generator<full_move> state::find_checks_impl<true>(std::vector<int>) const;
template std::vector<vec4> state::gen_movable_pieces_impl<false>(std::vector<int>) const;
template std::vector<vec4> state::gen_movable_pieces_impl<true>(std::vector<int>) const;
