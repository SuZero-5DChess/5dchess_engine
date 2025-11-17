
#include "hypercuboid.h"
#include "graph.h"


// for debug
#include <cassert>
#include <iostream>

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
            assert(false && "there shouldn't be a pass here");
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


std::tuple<HC_info, search_space> HC_info::build_HC(const state& s)
{
    std::map<int, int> line_to_axis; // map from timeline index to axis index
    std::vector<std::vector<semimove>> axis_coords; // axis_coords[i] is the set of all moves on i-th playable board
    HC universe;
    int sign = signum(s.new_line()); // sign for the new lines
    int new_axis, dimension;
    std::vector<std::set<int>> nonbranching_axes, branching_axes;
    auto [mandatory_timelines, optional_timelines, unplayable_timelines] = s.get_timeline_status();
    auto playable_timelines = concat_vectors(mandatory_timelines, optional_timelines);
    auto [present_t, present_c] = s.apparent_present();
    
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
            locs.push_back(physical_move{m, b_ptr});
        }
        for(vec4 p : departs_from[l])
        {
            jump_indices[p] = static_cast<int>(locs.size());
            // store the departing board after move is made
            std::shared_ptr<board> b_ptr = s.get_board(p.l(), p.t(), present_c)
                ->replace_piece(p.xy(), NO_PIECE);
            locs.push_back(departing_move{p, b_ptr});
        }
        for(full_move m : arrives_to[l])
        {
            // store the arriving board after move is made
            piece_t pic = s.get_piece(m.from, present_c);
            std::shared_ptr<board> b_ptr = s.get_board(m.to.l(), m.to.t(), present_c)
                ->replace_piece(m.to.xy(), pic);
            // use a temporary idx of -1, will be filled later
            locs.push_back(arriving_move{m, b_ptr, -1});
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
                assert(jump_indices.contains(p->m.from));
                p->idx = jump_indices[p->m.from];
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
            locs.push_back(arriving_move{m, b_ptr, jump_indices[m.from]});
        }
    }
    // replicate this axis max_branch times
    for(int i = 0; i < max_branch; i++)
    {
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
    HC_info info(s, line_to_axis, axis_coords, universe, sign, new_axis, dimension);
    
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
    search_space ss {{hc_n_lines}};
    for(int n = new_axis; n < dimension; n++)
    {
        hc_n_lines.axes[n] = non_null;
        ss.hcs.push_back(hc_n_lines);
    }
    return std::make_tuple(info, ss);
}


std::optional<point> HC_info::take_point(const HC &hc) const
{
    graph g(dimension);
    std::vector<int> must_include;
    // store a pair of departing/arriving move for each edge
    std::map<std::pair<int,int>, int> edge_refs;
    point result = std::vector<int>(dimension, -1);
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
                    if(!edge_refs.contains(std::make_pair(from_axis, n)))
                    {
                        g.add_edge(from_axis, n);
                        edge_refs[std::make_pair(from_axis, n)] = i;
                        edge_refs[std::make_pair(n, from_axis)] = loc.idx;
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
    std::cout << g.to_string();
    print_range("must include: ", must_include);
    print_range("intermediate result:", result);
    auto matching = g.find_matching(must_include);
    if(matching)
    {
        for(const auto& [u,v] : matching.value())
        {
            result[u] = edge_refs[std::make_pair(u,v)];
        }
        print_range("final result:", result);
        return std::optional<point>(result);
    }
    else
    {
        // matching exists
        return std::nullopt;
    }
}



std::optional<slice> HC_info::find_problem(const point &p, const HC& hc) const
{
    return jump_order_consistent(p, hc).or_else([this, &hc, &p]() {
        return test_present(p, hc).or_else([this, &hc, &p]() {
            return find_checks(p, hc);
        });
    });
}

std::optional<slice> HC_info::jump_order_consistent(const point &p, const HC& hc) const
{
    // throughout the search, maintain the jump_map of (l,t) ~> new_l
    // remember t, l are stored as the higher part of a vec4
    std::map<vec4, int> jump_map;
    
    for(int n = new_axis; n < dimension; n++)
    {
        const int xn = p[n];
        const semimove& loc = axis_coords[n][xn];
        if(std::holds_alternative<null_move>(loc))
        {
            continue;
        }
        assert(std::holds_alternative<arriving_move>(loc));
        const arriving_move arr = std::get<arriving_move>(loc);
        const vec4 from = arr.m.from, to = arr.m.to;
        /* case one: there is a branching move (l0,t0) -> (l',t'), but
         (l',t') is a playable board in which the corresponding played move
         is a none_move */
        const int m = line_to_axis.at(from.l());
        const int xm = p[m];
        const semimove& loc2 = axis_coords[m][xm];
        if(std::holds_alternative<null_move>(loc2))
        {
            null_move nm = std::get<null_move>(loc2);
            if(nm.tl == to.tl())
            {
                /* ban these combinations:
                    -[m,xm] the null_move on (l', t'); with
                    -[s] any branching move on axis n to (l', t') (which is a null_move)
                   i.e. all moves (l, t) -> (l',t') that are in nth axis of hc
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
                std::map<int, std::set<int>> fixed_axes {{n, s}, {m, {xm}}};
                return slice(fixed_axes);
            }
        }
        
        /* case two: there is a branching move (l,t) -> (l',t')
         but the source (l,t) is already registered in jump_map, i.e.
         there was a jump (l0,t0) -> (l,t) ~> (new_l0,t) */
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
            return slice(fixed_axes);
        }
        // maintain jump_map
        jump_map[from.tl()] = n;
    }
    return std::nullopt;
}

std::optional<slice> HC_info::test_present(const point &p, const HC& hc) const
{
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
                        return false; // trigger early return outside
                    new_present = newline_t;
                }
            }
            return true;
        };

        if(c) // white is playing
        {
            if (!check_newline(active_min, l_min, -1))
                return std::nullopt;
        }
        else // black is playing
        {
            if (!check_newline(active_max, l_max, 1))
                return std::nullopt;
        }
    }
    // step two: identify pass
    // the present cannot be shifted because `null_move` is played on a playable line
    // (that playable line becomes active after performing other moves)
    std::optional<int> pass_axis = std::nullopt;
    for(int l = active_min; l < active_max; l++)
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
            problem.fixed_axes[n] = late_jump;
        }
    }
    // handle the departures corresponding to leaves
    for(int n = 0; n < new_axis; n++)
    {
        if(early_jump_indices[n].size() > 0)
        {
            std::set<int> removed = set_minus(hc.axes[n], early_jump_indices[n]);
            problem.fixed_axes[n] = removed;
        }
    }
    return problem;
}

std::optional<slice> HC_info::find_checks(const point &p, const HC& hc) const
{
    auto [t, c] = s.get_present();
    moveseq mvs = to_action(p);
    state newstate = s;
    for(full_move mv : mvs)
    {
        bool flag = newstate.apply_move(mv);
        assert(flag && "failed to apply move here");
    }
    auto gen = newstate.find_checks();
    if(auto maybe_check = gen.first())
    {
        // there is a check
        // the slice to remove is a product of coordinates on certain axes
        full_move check = maybe_check.value();
        auto [path, sliding_type] = get_move_path(newstate, check, 1 - c);
        /* on axis for check.from.l():
         ban all moves that allows the same piece hostile piece remain in that position
         */
        int n1 = line_to_axis.at(check.from.l());
        std::set<int> not_taking;
        for(int i : hc.axes[n1])
        {
            semimove loc = axis_coords[n1][i];
            std::shared_ptr<board> newboard = extract_board(loc);
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
                    not_taking.insert(i);
                }
            }
            else if(newboard->get_piece(check.from.xy()) == s.get_piece(check.from, 1-c))
            {
                // non sliding pieces remains in same position
                not_taking.insert(i);
            }
        }
        /* on axis for check.to.l():
         ban all moves that leave a royal piece on check.to
         */
        int n2 = line_to_axis.at(check.to.l());
        std::set<int> expose_royal;
        for(int i : hc.axes[n2])
        {
            semimove loc = axis_coords[n2][i];
            std::shared_ptr<board> newboard = extract_board(loc);
            bool is_royal = pmask(check.to.xy()) & newboard->royal();
            if(is_royal)
                expose_royal.insert(i);
        }
        slice problem = {{{n1, not_taking}, {n2, expose_royal}}};
        /* on axes for checking path crossings: ban all moves, except for
         those (physical or arriving) brings a non-royal piece blocking the crossed square
         */
        for(vec4 crossed : path)
        {
            int n = line_to_axis.at(crossed.l());
            bitboard_t z = pmask(crossed.xy());
            std::set<int> not_blocking;
            for(int i : hc.axes[n1])
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
                not_blocking.insert(i);
            }
            problem.fixed_axes[n] = not_blocking;
        }
        assert(problem.contains(p));
        return problem;
    }
    return std::nullopt;
}

moveseq HC_info::to_action(const point &p) const
{
    std::vector<full_move> mvs;
    for(int i = 0; i < p.size(); i++)
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
    std::sort(mvs.begin(), mvs.end(), [sign=sign](full_move a, full_move b){
        return sign*a.to.l() < sign*b.to.l();
    });
    return mvs;
}



// ------------------------------------------------------------


generator<moveseq> HC_info::search(search_space ss) const
{
    std::cout << "HC_search::iterator::advance(): ";
    while(!ss.hcs.empty())
    {
        std::cout << "searching " << ss.to_string() << "\n";
        HC hc = ss.hcs.back();
        ss.hcs.pop_back();
        auto pt_opt = take_point(hc);
        if(pt_opt)
        {
            point pt = pt_opt.value();
            print_range("got point: ", pt);
            auto problem = find_problem(pt, hc);
            if(problem)
            {
                std::cout << "found problem: " << problem.value().to_string() << "\n";
                // remove the problematic slice from hc, and add the remaining to ss
                search_space new_ss = hc.remove_slice(problem.value());
                ss.concat(std::move(new_ss));
                std::cout << "countinue searching\n";
            }
            else
            {
                std::cout << "point is okay, removing it from this hc\n";
                co_yield to_action(pt);
                search_space new_ss = hc.remove_point(pt);
                ss.concat(std::move(new_ss));
            }
        }
        else
        {
            std::cout << "didn't secure any point in the first hypercuboid;\n";
            std::cout << "continue searching the remaining part.\n";
        }
    }
    std::cout << "search space is empty; abort.\n";
    co_return;
}
