#include "state.h"
#include <algorithm>
#include "utils.h"

state::state(multiverse mtv) : m(mtv)
{
    std::tie(number_activated, present, player) = m.get_present();
}

int state::new_line() const
{
    if(player == 0)
        return m.l_max + 1;
    else
        return m.l_min - 1;
}

bool state::can_submit() const
{
    auto [_, present_t, present_c] = m.get_present();
//    std::cout << present_t << " " << present_c << std::endl;
//    std::cout << present << " " << player << std::endl;
//    std::cout << (player != present_c) << (present != present_t);
    return player != present_c || present != present_t;
}

bool state::apply_move(full_move fm)
{
    bool flag;
    std::visit(overloads {
        [&](std::monostate)
        {
            // can present shift to opponent?
            if(!can_submit())
            {
                flag = false;
            }
            else
            {
                auto [_, present_t, present_c] = m.get_present();
                present = present_t;
                player  = present_c;
                flag = true;
            }
        },
        [&](std::tuple<vec4, vec4> data)
        {
            auto [p, d] = data;
            vec4 q = p+d;
            vector<vec4> moves = m.gen_piece_move(p, player);
            
            // is this a viable move? (not counting checking)
            if(!std::ranges::contains(moves, d))
            {
                flag = false;
                return;
            }
            // TODO: checking detection & castling passage detection
            
            // physical move, no time travel
            if(d.l() == 0 && d.t() == 0)
            {
                // TODO: detect en passant
                // TODO: detect castling
                const shared_ptr<board>& b_ptr = m.get_board(p.l(), p.t(), player);
                m.append_board(p.l(), b_ptr->move_piece(p.xy(), q.xy()));
            }
            // non-branching superphysical move
            else if(multiverse::tc_to_v(q.t(), player) == m.timeline_end[multiverse::l_to_u(q.l())])
            {
                const shared_ptr<board>& b_ptr = m.get_board(p.l(), p.t(), player);
                const piece_t& pic = static_cast<piece_t>(piece_name((*b_ptr)[p.xy()]));
                m.append_board(p.l(), b_ptr->replace_piece(p.xy(), NO_PIECE));
                const shared_ptr<board>& c_ptr = m.get_board(q.l(), q.t(), player);
                m.append_board(q.l(), c_ptr->replace_piece(q.xy(), pic));
            }
            //branching move
            else
            {
                const shared_ptr<board>& b_ptr = m.get_board(p.l(), p.t(), player);
                const piece_t& pic = static_cast<piece_t>(piece_name((*b_ptr)[p.xy()]));
                m.append_board(p.l(), b_ptr->replace_piece(p.xy(), NO_PIECE));
                const shared_ptr<board>& x_ptr = m.get_board(q.l(), q.t(), player);
                auto [t, c] = multiverse::v_to_tc(multiverse::tc_to_v(q.t(), player)+1);
                m.insert_board(new_line(), t, c, x_ptr->replace_piece(q.xy(), pic));
            }
            flag = true;
        }
    }, fm.data);
    return flag;
}
