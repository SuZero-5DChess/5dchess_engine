
#include "hypercuboid.h"
#include "graph.h"


// for debug
#include <cassert>
#include <iostream>
#define DEBUGMSG
#include "debug.h"

//generator<moveseq> search_legal_action(state s)
//{
//    
//}

std::shared_ptr<board> extract_board(const semimove& loc)
{
    return std::visit([](const auto& move) -> std::shared_ptr<board> {
        using T = std::decay_t<decltype(move)>;
        
        if constexpr (std::is_same_v<T, physical_move>
                   || std::is_same_v<T, arriving_move>
                   || std::is_same_v<T, departing_move>)
        {
            return move.b;
        }
        else
        {
            assert(false && "shouldn't be a pass here");
            return nullptr;
        }
    }, loc);
}

std::pair<int, int> extract_tl(const semimove& loc)
{
    vec4 p = std::visit(overloads {
        [](physical_move ll) {
            return ll.m.from.tl();
        },
        [](arriving_move ll) {
            return ll.m.to.tl();
        },
        [](departing_move ll) {
            return ll.from.tl();
        },
        [](null_move ll) {
            return ll.tl;
        }
    }, loc);
    return std::make_pair(p.t(), p.l());
}

/*
 gen_move_path: in state s, find the checking path
 return the checking path and sliding_type, where
 - sliding_type = 0 : non-sliding
 - sliding_type = 1 : rook move
 - sliding_type = 2 : bishop move
 - sliding_type = 3 : unicorn move
 - sliding_type = 4 : dragon move
 the endpoints are excluded in checking path
 */
std::tuple<std::vector<vec4>, int> get_move_path(const state &s, full_move fm, int c)
{
    const vec4 p = fm.from, q = fm.to, d = q - p;
    std::shared_ptr<board> b_ptr = s.get_board(p.l(), p.t(), c);
    if(b_ptr->sliding() & pmask(p.xy()))
    {
        // this piece is sliding, makes sense to talk about path
        std::vector<vec4> path;
        vec4 c = vec4(signum(d.x()), signum(d.y()), signum(d.t()), signum(d.l()));
        int sliding_type = c.dot(c);
#ifndef NDEBUG
        {
            assert(p!=q && sliding_type > 0 && sliding_type <= 4);
            std::set tester = {abs(d.l()), abs(d.t()), abs(d.x()), abs(d.y())};
            tester.erase(0);
            assert(tester.size() == 1 && "This doesn't look like a good sliding move");
        }
#endif
        for(vec4 r = p + c; r != q; r = r + c)
        {
            path.push_back(r);
        }
        return std::make_tuple(path, sliding_type);
    }
    else
    {
        return std::make_tuple(std::vector<vec4>(), 0);
    }
}

// test if king/royal queen with color c is under attack
bool has_physical_check(const board &b, bool c, vec4 p = {0,0,0,0})
{
    bitboard_t friendly =  c ? b.black() : b.white();
    for(int pos : marked_pos(b.royal() & friendly))
    {
        if([[maybe_unused]] auto x = b.is_under_attack(pos, c))
        {
            dprint("physical check", full_move(vec4(marked_pos(x)[0],p.tl()),vec4(pos, p.tl())));
            dprint(b.to_string());
            return true;
        }
    }
    dprint("no check for", c, "in");
    dprint(b.to_string());
    return false;
}

std::tuple<HC_info, search_space> HC_info::build_HC(const state& s)
{
    dprint("HC_info::build_HC()");
    std::map<int, int> line_to_axis; // map from timeline index to axis index
    std::vector<std::vector<semimove>> axis_coords; // axis_coords[i] is the set of all moves on i-th playable board
    HC universe;
    int sign = signum(s.new_line()); // sign for the new lines
    int new_axis, dimension;
    std::vector<std::set<int>> nonbranching_axes, branching_axes;
    auto [mandatory_timelines, optional_timelines, unplayable_timelines] = s.get_timeline_status();
    auto playable_timelines = concat_vectors(mandatory_timelines, optional_timelines);
    assert(!s.can_submit());
    auto [present_t, present_c] = s.get_present();
//    std::set<int> playable_timelines;
    
    // generate all moves, then split them into cases
    // for departing moves, we merge the moves that depart from the same coordinate
    std::map<int, std::vector<full_move>> arrives_to, stays_on;
    std::map<int, std::vector<vec4>> departs_from;
    // to track the corresponding departing moves for each arriving move
    std::map<vec4, int> jump_indices;
    
    for(vec4 from : s.gen_movable_pieces())
    {
        bool has_depart = false;
        for(const vec4 &to : s.gen_piece_move(from))
        {
            full_move m(from, to);
            //playable_timelines.insert(from.l()); //TODO: optimize
            if(from.tl() != to.tl())
            {
                if(!has_depart)
                {
                    departs_from[from.l()].push_back(m.from);
                }
                has_depart = true;
                arrives_to[to.l()].push_back(m);
            }
            else
            {
                stays_on[from.l()].push_back(m);
            }
        }
        
    }
    
    size_t estimate_size = 1 + arrives_to.size() + departs_from.size();
    
    // build nonbranching axes
    for(int l : playable_timelines)
    {
        std::vector<semimove> locs = {null_move{vec4(0,0,present_t,l)}};
        locs.reserve(estimate_size);
        for(full_move m : stays_on[l])
        {
            vec4 p = m.from, q = m.to;
            std::shared_ptr<board> b0_ptr = s.get_board(p.l(), p.t(), present_c);
            piece_t pic = b0_ptr->get_piece(p.xy());
            std::shared_ptr<board> b_ptr = b0_ptr
                ->replace_piece(p.xy(), NO_PIECE)
                ->replace_piece(q.xy(), pic);
            // filter physical checks in the very begining
            bool flag = has_physical_check(*b_ptr, present_c, p);
            if(!flag)
            {
                locs.push_back(physical_move{m, b_ptr});
            }
        }
        for(vec4 p : departs_from[l])
        {
            jump_indices[p] = static_cast<int>(locs.size());
            // store the departing board after move is made
            std::shared_ptr<board> b_ptr = s.get_board(p.l(), p.t(), present_c)
                ->replace_piece(p.xy(), NO_PIECE);
            bool flag = has_physical_check(*b_ptr, present_c, p);
            if(!flag)
                locs.push_back(departing_move{p, b_ptr});
        }
        for(full_move m : arrives_to[l])
        {
            // only store (possible) non-branching jump arrives
            auto [last_t, last_c] = s.get_timeline_end(m.to.l());
            if(m.to.t() == last_t && present_c == last_c)
            {
                assert(m.from.tl()!=m.to.tl());
                // store the arriving board after move is made
                piece_t pic = s.get_piece(m.from, present_c);
                std::shared_ptr<board> b_ptr = s.get_board(m.to.l(), m.to.t(), present_c)
                ->replace_piece(m.to.xy(), pic);
                //adprint("Was:", pic, "\n", s.get_board(m.to.l(), m.to.t(), present_c)->to_string());
                //adprint("After applying:", m, "\n", b_ptr->to_string());
                // use a temporary idx of -1, will be filled later
                bool flag = has_physical_check(*b_ptr, present_c);
                //adprint("Has physical check for",present_c,":",flag);
                if(!flag)
                    locs.push_back(arriving_move{m, b_ptr, -1});
            }
        }
        // save this axis
        locs.shrink_to_fit();
        line_to_axis[l] = static_cast<int>(axis_coords.size());
        axis_coords.push_back(std::move(locs));
    }
    // fill the idx of arriving moves 
    for(int n = 0; n < axis_coords.size(); n++)
    {
        for(int i = 0; i < axis_coords[n].size(); i++)
        {
            semimove& loc = axis_coords[n][i];
            if(auto* p = std::get_if<arriving_move>(&loc))
            {
                if(jump_indices.contains(p->m.from))
                {
                    p->idx = jump_indices[p->m.from];
                }
                else
                {
                    dprint("removing ghost arrive at axis:", n, "no:", i);
                    axis_coords[n].erase(axis_coords[n].begin() + i);
                }
            }
        }
    }
    
    new_axis = static_cast<int>(axis_coords.size());

    // build branching axes
    int max_branch = 0;
    std::vector<semimove> locs = {null_move{vec4(0,0,present_t,0)}};
    // original is `Pass undefined`
    for(const auto& [l, froms] : departs_from)
    {
        // determine the number of branching axes
        if(!froms.empty())
        {
            max_branch++;
        }
        // simutaneously, collect all branching moves
        for(const full_move& m : arrives_to[l])
        {
            piece_t pic = s.get_piece(m.from, present_c);
            std::shared_ptr<board> b_ptr = s.get_board(m.to.l(), m.to.t(), present_c)
                ->replace_piece(m.to.xy(), pic);
            assert(jump_indices.contains(m.from));
            bool flag = has_physical_check(*b_ptr, present_c);
            if(!flag)
                locs.push_back(arriving_move{m, b_ptr, jump_indices[m.from]});
        }
    }
    // replicate this axis max_branch times
    const int new_l = s.new_line();
    for(int i = 0; i < max_branch; i++)
    {
        assert(!line_to_axis.contains(new_l+sign*i));
        line_to_axis[new_l+sign*i] = new_axis + i;
        axis_coords.push_back(locs);
    }
    dimension = static_cast<int>(axis_coords.size());
    
    // build the whole space
    universe.axes.reserve(dimension);
    for(int n = 0; n < dimension; n++)
    {
        std::set<int> coords;
        // on nth dimension, the hypercube has coordinates 0, 1, ..., m avialible
        // which corresponds to axis_coords[n][0], axis_coords[n][1], ...
        for(int i = 0; i < axis_coords[n].size(); i++)
        {
            coords.insert(coords.end(), i);
        }
        universe.axes.push_back(std::move(coords));
    }
    HC_info info(s, line_to_axis, axis_coords, universe, sign, new_axis, dimension, mandatory_timelines);
    
    // split the search space by number of branches
    // TODO: move this part into search() 
    HC hc_n_lines = universe;
    std::set<int> singleton = {0}, non_null;
    if(new_axis < dimension)
    {
        for(int i = 1; i < axis_coords[new_axis].size(); i++)
        {
            non_null.insert(non_null.end(), i);
        }
        // non_null = {1,2,...,number of branching moves}
        std::fill(hc_n_lines.axes.begin() + new_axis,
                  hc_n_lines.axes.end(), singleton);
    }
    search_space ss{{hc_n_lines}};
    for(int n = new_axis; n < dimension; n++)
    {
        hc_n_lines.axes[n] = non_null;
        ss.hcs.push_front(hc_n_lines); // prefer lesser branching moves
    }
    return std::make_tuple(info, ss);
}


std::optional<point> HC_info::take_point(const HC &hc) const
{
    dprint("take_point()", hc.to_string());
    graph g(dimension);
    std::vector<int> must_include;
    // store a pair of departing/arriving move for each edge
    // edge_refs[{p,q}] = the corresponding move on axis p
    std::map<std::pair<int,int>, int> edge_refs;
    point result = std::vector<int>(dimension, -1);
    //build edge_refs and fill default physical moves in result
    for(int n = 0; n < dimension; n++)
    {
        bool has_nonjump = false;
        for(int i : hc[n])
        {
            const semimove& loc = axis_coords[n][i];
            std::visit(overloads {
                [&](const physical_move& loc) {
                    if(!has_nonjump)
                    {
                        has_nonjump = true;
                        result[n] = i;
                    }
                },
                [&](const arriving_move& loc) {
                    int from_axis = line_to_axis.at(loc.m.from.l());
                    if(!hc.axes[from_axis].contains(loc.idx))
                    {
                        return;
                    }
                    if(!edge_refs.contains(std::make_pair(from_axis, n)))
                    {
                        g.add_edge(from_axis, n);
//                        std::cout << loc.m << "\n";
//                        std::cout << g.to_string();
                        assert(from_axis!=n);
                        edge_refs[std::make_pair(from_axis, n)] = loc.idx;
                        edge_refs[std::make_pair(n, from_axis)] = i;
                        assert(loc.idx != -1);
                    }
                },
                [](const departing_move& loc) {},
                [&](const null_move& loc) {
                    if(!has_nonjump)
                    {
                        has_nonjump = true;
                        result[n] = i;
                    }
                },
            }, loc);
        }
        if(!has_nonjump)
        {
            must_include.push_back(n);
        }
    }
    dprint("constructed", g.to_string());
    dprint("must include: ", range_to_string(must_include));
    dprint("intermediate result:", range_to_string(result));
    auto matching = g.find_matching(must_include);
    if(matching)
    {
        dprint("matching", range_to_string(*matching));
        for(const auto& [u,v] : matching.value())
        {
            result[u] = edge_refs[std::make_pair(u,v)];
            result[v] = edge_refs[std::make_pair(v,u)];
        }
#ifndef NDEBUG
        for(int i:result)
        {
            assert(i != -1 && "some axis is still null");
        }
#endif
        dprint("final result:", range_to_string(result));
        assert(hc.contains(result));
        for(int n = 0; n < result.size(); n++)
        {
            int i = result[n];
            dprint("n=",n,",i=",i,",loc=",show_semimove(axis_coords[n][i]));
        }
        return std::optional<point>(result);
    }
    else
    {
        dprint("no match");
        return std::nullopt;
    }
}



std::optional<slice> HC_info::find_problem(const point &p, const HC& hc) const
{
    std::optional<slice> problem = jump_order_consistent(p, hc).or_else([this, &hc, &p]() {
        return test_present(p, hc).or_else([this, &hc, &p]() {
            return find_checks(p, hc);
        });
    });
    return problem;
}

std::optional<slice> HC_info::jump_order_consistent(const point &p, const HC& hc) const
{
    dprint("jump_order_consistent()");
    /* throughout the search, maintain the jump_map of (l, t) => new_l
    so p[new_l] is a arriving move from (l0, t0) that jumps to (l, t) which create a branch
    remember t, l are stored as the higher part of a vec4
     */
    std::map<vec4, int> jump_map;
    
//    if(p == std::vector<int>{12,7,11,3})
//    {
//        std::cout << "ksdnfknk" << std::endl;
//    }
    
    for(int n = new_axis; n < dimension; n++)
    {
        // within this loop, n is the axis for new_l
        const int in = p[n];
        const semimove& loc = axis_coords[n][in];
        if(std::holds_alternative<null_move>(loc))
        {
            continue;
        }
        assert(std::holds_alternative<arriving_move>(loc));
        const arriving_move arr = std::get<arriving_move>(loc);
        const vec4 from = arr.m.from, to = arr.m.to;
        /* case one: there is a branching move (l0,t0) >> (l',t') ~> new_l, but
         (l',t') is a playable board in which the corresponding played move
         is a pass */
        const int m = line_to_axis.at(to.l());
        const int im = p[m];
        const semimove& loc2 = axis_coords[m][im];
        if(std::holds_alternative<null_move>(loc2))
        {
            null_move nm = std::get<null_move>(loc2);
            if(nm.tl == to.tl())
            {
                /* ban these combinations:
                    -[m,xm] the null_move on (l', t'); with
                    -[s] any branching move on axis n to (l', t') (which is a null_move)
                   i.e. all moves >> (l',t') then creates branch new_l
                */
                std::set<int> s;
                for(int i : hc[n])
                {
                    const semimove& loc3 = axis_coords[n][i];
                    if(std::holds_alternative<arriving_move>(loc3))
                    {
                        vec4 to3 = std::get<arriving_move>(loc3).m.to;
                        if(to3.tl() == to.tl())
                        {
                            s.insert(i);
                        }
                    }
                }
                std::map<int, std::set<int>> fixed_axes {{n, s}, {m, {im}}};
                slice problem(fixed_axes);
                dprint("case one; point:", range_to_string(p));
                dprint("problem", problem.to_string());
                assert(problem.contains(p));
                return problem;
            }
        }
        //dprint(from.tl());
        /* case two: there is a branching move (l,t) -> (l',t')
         but the source (l,t) is already registered in jump_map, i.e.
         there was a jump (l0,t0) -> (l,t) ~> (new_l0,t)
         where new_l0 happens latter than new_l
         */
        vec4 critical_tl = from.tl();
        if(jump_map.contains(critical_tl))
        {
            /* ban these combinations:
                -[s1] on axis n, any move starts from (l,t); with
                -[s2] on axis for new_l0, any move goes to (l,t)
            */
            int axis_branch = jump_map[critical_tl];
            std::set<int> s1, s2;
            for(int i : hc.axes[n])
            {
                const semimove& l1 = axis_coords[n][i];
                if(std::holds_alternative<arriving_move>(l1))
                {
                    vec4 from1 = std::get<arriving_move>(l1).m.from;
                    if(from1.tl() == critical_tl)
                    {
                        s1.insert(i);
                    }
                }
            }
            for(int i : hc.axes[axis_branch])
            {
                const semimove& l2 = axis_coords[axis_branch][i];
                if(std::holds_alternative<arriving_move>(l2))
                {
                    vec4 to2 = std::get<arriving_move>(l2).m.to;
                    if(to2.tl() == critical_tl)
                    {
                        s2.insert(i);
                    }
                }
            }
            std::map<int, std::set<int>> fixed_axes {{n, s1}, {axis_branch, s2}};
            slice problem(fixed_axes);
            dprint("case two; point:", range_to_string(p));
            dprint("problem", problem.to_string());
            assert(problem.contains(p));
            return problem;
        }
        // maintain jump_map
        jump_map[to.tl()] = n;
    }
    dprint("no problem");
    return std::nullopt;
}

std::optional<slice> HC_info::test_present(const point &p, const HC& hc) const
{
    dprint("test_present()");
    const auto [old_present, c] = s.get_present();
    const auto [l_min, l_max] = s.get_lines_range();
    auto [active_min, active_max] = s.get_active_range();
    // step one: find the new present
    int new_present = old_present;
    for(int n = new_axis; n < dimension; n++)
    {
        // for all beanching moves
        int i = p[n];
        // present may need to move to the time of this arrive
        semimove loc = axis_coords[n][i];
        if(std::holds_alternative<null_move>(loc))
            continue; // null_move's don't change present
        auto [t, l] = extract_tl(loc);
        if(t < new_present)
        {
            new_present = t;
        }
        // there is probably a newly activated line after this branching move
        // which moves the present
        auto check_newline = [&](int &active, int limit, int delta) -> bool
        {
            if ((delta < 0 && limit < active) || (delta > 0 && limit > active))
            {
                active += delta;
                const auto [newline_t, newline_c] = s.get_timeline_end(active);
                if (newline_t < new_present)
                {
                    /* if the newly activated line is prior to current present
                     and belongs to the opponent, then we have already shifted
                     the present; no problem
                     */
                    if (newline_c != c)
                    {
                        return false; // trigger early return outside
                    }
                    new_present = newline_t;
                }
            }
            return true;
        };

        if(c) // white is playing //TODO: REALLY?
        {
            if (!check_newline(active_min, l_min, -1))
            {
                dprint("shifted present; no problem");
                return std::nullopt;
            }
        }
        else // black is playing
        {
            if (!check_newline(active_max, l_max, 1))
            {
                dprint("shifted present; no problem");
                return std::nullopt;
            }
        }
    }
    // step two: identify pass
    // the present cannot be shifted because `null_move` is played on a playable line
    // (that playable line becomes active after performing other moves)
    std::optional<int> pass_axis = std::nullopt;
    for(int l : mandatory_lines)
    {
        int n = line_to_axis.at(l);
        semimove loc = axis_coords[n][p[n]];
        if(std::holds_alternative<null_move>(loc))
        {
            pass_axis = n;
            break;
        }
    }
    if(!pass_axis.has_value())
    {
        dprint("no problem");
        return std::nullopt;
    }
    // step three: generate problem
    /* ban these combinations:
     - on pass_axis: null_move
     - on other axes: not a branching move that travels further back than new_present
        track them by iterating all arrives on new line and deal with their idx
     */
    slice problem {{{pass_axis.value(), {0}}}};
    std::vector<std::set<int>> early_jump_indices(new_axis);
    for(int n = new_axis; n < dimension; n++)
    {
        if(n==pass_axis) [[unlikely]]
            continue;
        std::set<int> late_jump = hc.axes[n];
        for(int i : hc.axes[n])
        {
            semimove loc = axis_coords[n][i];
            auto [t, l] = extract_tl(loc);
            if(t < new_present) [[unlikely]] // early jump
            {
                late_jump.erase(i);
                assert(std::holds_alternative<arriving_move>(loc));
                arriving_move am = std::get<arriving_move>(loc);
                int from_axis = line_to_axis.at(am.m.from.l());
                early_jump_indices[from_axis].insert(am.idx);
            }
        }
        if(late_jump.size() < hc.axes[n].size())
        {
            // only need to fix this axis when not all coordinates are removed
            dprint("on axis", n, "remove late jump", range_to_string(late_jump));
            problem.fixed_axes[n] = late_jump;
        }
    }
//    // handle the departures corresponding to leaves
//    for(int n = 0; n < new_axis; n++)
//    {
//        if(early_jump_indices[n].size() > 0)
//        {
//            std::set<int> removed = set_minus(hc.axes[n], early_jump_indices[n]);
//
//            problem.fixed_axes[n] = removed;
//        }
//    }
    dprint("point:", range_to_string(p));
    dprint("problem", problem.to_string());
    assert(problem.contains(p));
    return problem;
}

std::optional<slice> HC_info::find_checks(const point &p, const HC& hc) const
{
    dprint("HC_info::find_checks()");
    auto [t, c] = s.get_present();
    moveseq mvs = to_action(p);
    state newstate = s;
#ifdef DEBUGMSG
    std::string mvsstr;
#endif
    for(full_move mv : mvs)
    {
#ifdef DEBUGMSG
        mvsstr += s.pretty_move(mv) + " ";
#endif
        bool flag = newstate.apply_move(mv);
        assert(flag && "failed to apply move here");
    }
    bool flag = newstate.submit();
    //auto [new_present, new_color] = newstate.get_present();
    assert(flag && "failed to submit here");
    dprint("after applying moves:", mvsstr, newstate.to_string());
    dprint("c=", c);
    auto gen = newstate.find_checks(!c);
//    if(p == std::vector<int>{12, 7, 1, 13})
//    {
//        //std::cout << newstate.to_string();
//        for(auto m : newstate.find_checks(!c))
//            std::cout << m << std::endl;
//    }
    if(auto maybe_check = gen.first())
    {
        // there is a check
        // the slice to remove is a product of coordinates on certain axes
        full_move check = maybe_check.value();
        dprint("found check: ", check);
        assert(check.from.tl() != check.to.tl() && "physical checks should have already removed");
        auto [path, sliding_type] = get_move_path(newstate, check, !c);
//        std::cout << std::endl;
//        dprint(s.get_board(check.from.l(), check.from.t(), !c));
        /* on axis for check.from.l():
         ban all moves that allows the same piece hostile piece remain in that position
         */
        int n1 = line_to_axis.at(check.from.l());
        std::set<int> not_taking;
        for(int i : hc.axes[n1])
        {
            semimove loc = axis_coords[n1][i];
            if(std::holds_alternative<null_move>(loc))
            {
                continue;
            }
            std::shared_ptr<board> newboard = extract_board(loc);
            dprint(n1, i, sliding_type, show_semimove(loc));
            if(sliding_type)
            {
                bitboard_t bb = 0;
                switch(sliding_type)
                {
                    case 1:
                        bb = newboard->lrook();
                        break;
                    case 2:
                        bb = newboard->lbishop();
                        break;
                    case 3:
                        bb = newboard->lunicorn();
                        break;
                    case 4:
                        bb = newboard->ldragon();
                        break;
                    default:
                        assert(false && "wrong sliding type infered");
                }
                if(pmask(check.from.xy()) & bb)
                {
                    dprint("axis", n1, "not taking (sliding)", i);
                    not_taking.insert(i);
                }
            }
            else if(newboard->get_piece(check.from.xy()) == newstate.get_piece(check.from, !c))
            {
                // non sliding pieces remains in same position
                dprint("axis", n1, "not taking (untouched)", i);
                not_taking.insert(i);
            }
            //TODO: debug here
        }
        /* on axis for check.to.l():
         ban all moves that leave a royal piece on check.to
         */
        int n2 = line_to_axis.at(check.to.l());
        std::set<int> expose_royal;
        for(int i : hc.axes[n2])
        {
            semimove loc = axis_coords[n2][i];
            std::shared_ptr<board> newboard;
            if(std::holds_alternative<null_move>(loc))
            {
                continue;
            }
            else
            {
                newboard = extract_board(loc);
            }
            //dprint(n2, i, show_semimove(loc));
            bool is_royal = pmask(check.to.xy()) & newboard->royal();
            if(is_royal)
            {
                dprint("axis", n2, "expose royal", i);
                expose_royal.insert(i);
            }
        }
        slice problem = {{{n1, not_taking}, {n2, expose_royal}}};
        dprint(n1, range_to_string(not_taking), n2, range_to_string(expose_royal));
        /* on axes for checking path crossings: ban all moves, except for
         those (physical or arriving) brings a non-royal piece blocking the crossed square
         */
        for(vec4 crossed : path)
        {
            int n = line_to_axis.at(crossed.l());
            bitboard_t z = pmask(crossed.xy());
            std::set<int> not_blocking;
            for(int i : hc.axes[n])
            {
                semimove loc = axis_coords[n][i];
                if(std::holds_alternative<physical_move>(loc))
                {
                    if(std::get<physical_move>(loc).m.to == crossed)
                    {
                        bool is_royal = z & std::get<physical_move>(loc).b->royal();
                        if(!is_royal)
                            continue;
                    }
                }
                else if (std::holds_alternative<arriving_move>(loc))
                {
                    if(std::get<arriving_move>(loc).m.to == crossed)
                    {
                        bool is_royal = z & std::get<arriving_move>(loc).b->royal();
                        if(!is_royal)
                            continue;
                    }
                }
                dprint("axis", n, "not blocking", i);
                not_blocking.insert(i);
            }
            problem.fixed_axes[n] = not_blocking;
        }
        dprint("point:", range_to_string(p));
        dprint("problem", problem.to_string());
        assert(problem.contains(p));
        return problem;
    }
    dprint("no checks found");
    return std::nullopt;
}

moveseq HC_info::to_action(const point &p) const
{
    std::vector<full_move> mvs;
//    std::vector<int> lines;
//    lines.reserve(line_to_axis.size());
//
//    for (auto &[k, _] : line_to_axis)
//        lines.push_back(k);
//
//    if (s.get_present().second)
//        std::reverse(lines.begin(), lines.end());
//    for(int i = 0; i < p.size(); i++)
//    {
//        semimove loc = axis_coords[i][p[i]];
//        if(std::holds_alternative<physical_move>(loc))
//        {
//            mvs.push_back(std::get<physical_move>(loc).m);
//        }
//        else if(std::holds_alternative<arriving_move>(loc))
//        {
//            mvs.push_back(std::get<arriving_move>(loc).m);
//        }
//    }
//    std::sort(mvs.begin(), mvs.end(), [sign=sign](full_move a, full_move b){
//        return sign*a.to.l() < sign*b.to.l();
//    });
    
    for(const auto [l,i] : line_to_axis)
    {
        semimove loc = axis_coords[i][p[i]];
        if(std::holds_alternative<physical_move>(loc))
        {
            mvs.push_back(std::get<physical_move>(loc).m);
        }
        else if(std::holds_alternative<arriving_move>(loc))
        {
            mvs.push_back(std::get<arriving_move>(loc).m);
        }
    }
    auto [t,c] = s.get_present();
    if(c)
    {
        std::reverse(mvs.begin(), mvs.end());
    }
    return mvs;
}



// ------------------------------------------------------------


generator<moveseq> HC_info::search(search_space ss) const
{
    adprint("begining search: ", ss.to_string());
    while(!ss.hcs.empty())
    {
        HC hc = ss.hcs.back();
        adprint("searching ", hc.to_string());
        ss.hcs.pop_back();
        auto pt_opt = take_point(hc);
        if(pt_opt)
        {
            point pt = pt_opt.value();
            print_range("got point: ", pt);
            auto problem = find_problem(pt, hc);
            if(problem)
            {
                adprint("found problem:", problem.value().to_string());
                // remove the problematic slice from hc, and add the remaining to ss
                search_space new_ss = hc.remove_slice(problem.value());
                // make sure when a leave is removed, so is the corresponding arrive
                adprint("removed problem, continue search:", new_ss.to_string());
                ss.concat(std::move(new_ss));
            }
            else
            {
                adprint("point is okay, removing it from this hc");
                co_yield to_action(pt);
                search_space new_ss = hc.remove_point(pt);
                adprint("removed point, continue search:", new_ss.to_string());
                ss.concat(std::move(new_ss));
            }
        }
        else
        {
            dprint("didn't secure any point in the first hypercuboid;");
            dprint("continue searching the remaining part");
        }
    }
    dprint("search space is empty; finish.");
    co_return;
}

std::vector<moveseq> HC_info::search1(search_space ss) const
{
    std::vector<moveseq> result;
    adprint("begining search: ", ss.to_string());
    while(!ss.hcs.empty())
    {
        HC hc = ss.hcs.back();
        adprint("searching ", hc.to_string());
        ss.hcs.pop_back();
        auto pt_opt = take_point(hc);
        if(pt_opt)
        {
            point pt = pt_opt.value();
            print_range("got point: ", pt);
            auto problem = find_problem(pt, hc);
            if(problem)
            {
                adprint("found problem:", problem.value().to_string());
                // remove the problematic slice from hc, and add the remaining to ss
                search_space new_ss = hc.remove_slice(problem.value());
                // make sure when a leave is removed, so is the corresponding arrive
                adprint("removed problem, continue search:", new_ss.to_string());
                ss.concat(std::move(new_ss));
            }
            else
            {
                adprint("point is okay, removing it from this hc");
                result.push_back(to_action(pt));
                search_space new_ss = hc.remove_point(pt);
                adprint("removed point, continue search:", new_ss.to_string());
                ss.concat(std::move(new_ss));
            }
        }
        else
        {
            dprint("didn't secure any point in the first hypercuboid;");
            dprint("continue searching the remaining part");
        }
    }
    dprint("search space is empty; finish.");
    return result;
}

