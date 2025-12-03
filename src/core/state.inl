
template<uint16_t FLAGS>
std::string state::pretty_move(full_move fm, piece_t pt) const
{
    static_assert((FLAGS & ~SHOW_ALL) == 0, "Invalid FLAGS for pretty_move");
    std::ostringstream oss;
    vec4 p = fm.from, q = fm.to;
    oss << m->pretty_lt(p.tl());
    piece_t pic = to_white(piece_name(get_piece(p, player)));
    if constexpr(FLAGS & SHOW_PAWN)
    {
        oss << pic;
    }
    else
    {
        if(pic != PAWN_W)
        {
            oss << pic;
        }
    }
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
        if constexpr(FLAGS & SHOW_CAPTURE)
        {
            if(get_piece(q, player) != NO_PIECE)
            {
                oss << "x";
            }
        }
        if constexpr(FLAGS & SHOW_RELATIVE)
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
        if constexpr(FLAGS & SHOW_CAPTURE)
        {
            if(get_piece(q, player) != NO_PIECE)
            {
                oss << "x";
            }
        }
    }
    oss << static_cast<char>(q.x() + 'a') << static_cast<char>(q.y() + '1');
    if constexpr(FLAGS & SHOW_PROMOTION)
    {
        if((pic == PAWN_W) && (q.y() == (player ? 0 : (m->get_board_size().second - 1))))
        {
            oss << "=" << pt;
        }
    }
    return oss.str();
}

template<uint16_t FLAGS>
std::string state::pretty_action(action act) const
{
    state s = *this;
    std::string ans = "";
    for(auto [m, pt] : act.get_moves())
    {
        ans += s.pretty_move<state::SHOW_CAPTURE>(m) + " ";
        s.apply_move(m, pt);
    }
    if(!ans.empty())
    {
        ans.pop_back();
    }
    return ans;
}
