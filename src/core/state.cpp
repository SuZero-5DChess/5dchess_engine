#include "state.h"
#include <algorithm>
#include <cassert>
#include "utils.h"

#include "pgnparser.h"

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

std::optional<full_move> state::parse_pgn(std::string move)
{
    //parse as a physical move
    auto parsed_physical_move = pgnparser(move).parse_physical_move();
    std::vector<full_move> matched;
    std::vector<full_move> pawn_move_matched;
    std::optional<full_move> fm;
    //if this is indeed a physical move
    if(parsed_physical_move)
    {
        // for all physical moves avilable in current state
        for(vec4 p : gen_movable_pieces())
        {
            char piece = to_white(piece_name(get_piece(p, player)));
            bitboard_t bb = player ? m->gen_physical_moves<true>(p) : m->gen_physical_moves<false>(p);
            for(int pos : marked_pos(bb))
            {
                vec4 q(pos, p.tl());
                full_move fm(p,q);
                // test if this physical move matches any of them
                std::string full_notation = pretty_move(fm, player);
                auto full = pgnparser(full_notation).parse_physical_move();
                assert(full.has_value());
                bool match = pgnparser::match_physical_move(*parsed_physical_move, *full);
                if(match)
                {
                    matched.push_back(fm);
                    if(piece == PAWN_W)
                    {
                        pawn_move_matched.push_back(fm);
                    }
                }
            }
        }
        if(matched.size()==1)
        {
            // if there is exactly one match, we are good
            fm = matched[0];
        }
        else if(pawn_move_matched.size() == 1)
        {
            /* if there are more than one match, test if it this can be
             parsed as the unique pawn move
             */
            fm = pawn_move_matched[0];
        }
    }
    else
    {
        // do the same for superphysical moves
        auto parsed_sp_move = pgnparser(move).parse_superphysical_move();
//        std::cerr << "original " << parsed_sp_move << std::endl;
        for(vec4 p : gen_movable_pieces())
        {
            char piece = to_white(piece_name(get_piece(p, player)));
            auto gen = player ? m->gen_superphysical_moves<true>(p) : m->gen_superphysical_moves<false>(p);
            for(const auto& [p0, bb] : gen)
            {
                for(int pos : marked_pos(bb))
                {
                    vec4 q(pos, p0);
                    full_move fm(p,q);
                    // test if this physical move matches any of them
                    std::string full_notation = pretty_move(fm, player);
                    auto full = pgnparser(full_notation).parse_superphysical_move();
                    assert(full.has_value());
                    bool match = pgnparser::match_superphysical_move(*parsed_sp_move, *full);
//                    std::cout << (match ? "match" : "no fit") << ":\t";
//                    std::cout << full_notation << "\n" << *full << "\n";
                    if(match)
                    {
                        matched.push_back(fm);
                        if(piece == PAWN_W)
                        {
                            pawn_move_matched.push_back(fm);
                        }
                    }
                }
            }
        }
        if(matched.size()==1)
        {
            fm = matched[0];
        }
        else if(pawn_move_matched.size() == 1)
        {
            fm = pawn_move_matched[0];
        }
    }
    return fm;
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

template<bool RELATIVE>
std::string state::pretty_move(full_move fm, int c) const
{
    std::ostringstream oss;
    vec4 p = fm.from, q = fm.to;
    oss << m->pretty_lt(p.tl());
    oss << to_white(piece_name(get_piece(p, c)));
    oss << static_cast<char>(p.x() + 'a') << static_cast<char>(p.y() + '1');
    if(p.tl() != q.tl())
    {
//        std::cout << "p=" << p << "\t q=" << q << "\t";        // superphysical move
        if(std::pair{q.t(), player} < get_timeline_end(q.l()))
        {
            oss << ">>";
        }
        else
        {
            oss << ">";
        }
        if(get_piece(q, c) != NO_PIECE)
        {
            oss << "x";
        }
        if constexpr(RELATIVE)
        {
            vec4 d = q - p;
            auto show_diff = [&oss](int w){
                if(w>0)
                    oss << "+" << w;
                else if(w<0)
                    oss << "-" << (-w);
                else
                    oss << "=";
            };
            oss << "$(L";
            show_diff(d.x());
            oss << "T";
            show_diff(d.y());
            oss << ")";
        }
        else
        {
            oss << m->pretty_lt(q.tl());
        }
    }
    else
    {
        //physical move
        if(get_piece(q, c) != NO_PIECE)
        {
            oss << "x";
        }
    }
    oss << static_cast<char>(q.x() + 'a') << static_cast<char>(q.y() + '1');
    //TODO: promotion
    return oss.str();
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

template std::string state::pretty_move<false>(full_move, int) const;
template std::string state::pretty_move<true>(full_move, int) const;
