#include "hypercuboid.h"
#include "graph.h"

// for debug
#include <cassert>
#include <iostream>


HC_search HC_search::build_HC(const state& s)
{
    std::map<int, int> line_to_axis; // map from timeline index to axis index
    std::vector<std::vector<AL>> axis_coords; // axis_coords[i] is the set of all moves on i-th playable board
    HC universe;
    int sign; // sign for the new lines
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
    
    for(const auto &[tl1, bb1] : s.gen_movable_pieces())
    {
        for(int pos1 : marked_pos(bb1))
        {
            bool has_depart = false;
            vec4 from(pos1, tl1);
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
    }
    
    size_t estimate_size = 1 + arrives_to.size() + departs_from.size();
    
    // build nonbranching axes
    for(int l : playable_timelines)
    {
        std::vector<AL> locs = {null_move{vec4(0,0,present_t,l)}};
        locs.reserve(estimate_size);
        for(full_move m : stays_on[l])
        {
            locs.push_back(physical_move{m});
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
            AL& loc = axis_coords[n][i];
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
    std::vector<AL> locs;
    locs.push_back(null_move{vec4(0,0,present_t,0)}); // original is `Pass undefined` (?)
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
    universe.axes.reserve(axis_coords.size());
    for(int n = 0; n < axis_coords.size(); n++)
    {
        std::set<int> s;
        // on nth dimension, the hypercube has coordinates 0, 1, ..., m avialible
        // which corresponds to axis_coords[n][0], axis_coords[n][1], ...
        for(int i = 0; i < axis_coords[n].size(); i++)
        {
            s.insert(s.end(), i);
        }
        universe.axes.push_back(std::move(s));
    }
    return HC_search(s, line_to_axis, axis_coords, universe, sign, new_axis, dimension);
}


std::optional<point> HC_search::take_point(const HC &hc) const
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
            const AL& loc = axis_coords[n][i];
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



std::optional<slice> HC_search::find_problem(const point &p, const HC& hc) const
{
    return jump_order_consistent(p, hc).or_else([this, &hc, &p]() {
        return test_present(p, hc).or_else([this, &hc, &p]() {
            return find_checks(p, hc);
        });
    });
}

std::optional<slice> HC_search::jump_order_consistent(const point &p, const HC& hc) const
{
    // throughout the search, maintain the jump_map of (l,t) ~> new_l
    // remember t, l are stored as the higher part of a vec4
    std::map<vec4, int> jump_map;
    
    for(int n = new_axis; n < dimension; n++)
    {
        const int xn = p[n];
        const AL& loc = axis_coords[n][xn];
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
        const AL& loc2 = axis_coords[m][xm];
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
                    const AL& loc3 = axis_coords[n][i];
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
                const AL& l1 = axis_coords[n][i];
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
                const AL& l2 = axis_coords[axis_branch][i];
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

std::optional<slice> HC_search::test_present(const point &p, const HC& hc) const
{
    return std::optional<slice>();
}

std::optional<slice> HC_search::find_checks(const point &p, const HC& hc) const
{
    return std::optional<slice>();
}

moveseq HC_search::to_action(const point &p) const
{
    std::vector<full_move> mvs;
    for(int i = 0; i < p.size(); i++)
    {
        AL loc = axis_coords[i][p[i]];
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



// -------------------------------------

void HC_search::iterator::advance()
{
    std::cout << "HC_search::iterator::advance(): ";
    if(ss.hcs.empty())
    {
        std::cout << "search space is empty; abort.\n";
        finished = true;
        return;
    }
    std::cout << "searching " << ss.to_string() << "\n";
    HC hc = ss.hcs.back();
    ss.hcs.pop_back();
    auto pt_opt = parent->take_point(hc);
    if(pt_opt)
    {
        point pt = pt_opt.value();
        print_range("got point: ", pt);
        auto problem = parent->find_problem(pt, hc);
        if(problem)
        {
            std::cout << "found problem: " << problem.value().to_string() << "\n";
            // remove the problematic slice from hc, and add the remaining to ss
            search_space new_ss = hc.remove_slice(problem.value());
            ss.concat(std::move(new_ss));
            std::cout << "countinue searching\n";
            advance(); // rely on tail recursion to continue searching
        }
        else
        {
            std::cout << "point is okay, removing it from this hc\n";
            current_result = parent->to_action(pt);
            search_space new_ss = hc.remove_point(pt);
            ss.concat(std::move(new_ss));
            std::cout << "search finished.\n";
        }
    }
    else
    {
        std::cout << "didn't secure any point in the first hypercuboid;\n";
        std::cout << "continue searching the remaining part.\n";
        advance();
    }
}

// below are standard code for iterators

HC_search::iterator::iterator(const HC_search *p, bool f): parent{p}, ss{{p->universe}}, finished{f}
{
    //advance();
}

HC_search::iterator HC_search::iterator::operator++()
{
    if(current_result.empty())
    {
        advance();
    }
    current_result.clear();
    return *this;
}

bool HC_search::iterator::operator==(const iterator &other) const
{
    return this->finished && other.finished;
}

bool HC_search::iterator::operator!=(const iterator &other) const
{
    return !(*this == other);
}

const moveseq& HC_search::iterator::operator*()
{
    if(current_result.empty())
    {
        advance();
    }
    return current_result;
}

HC_search::iterator HC_search::begin()
{
    return iterator(this, false);
}

HC_search::iterator HC_search::end()
{
    return iterator(this, true);
}
