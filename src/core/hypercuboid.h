// created by ftxi on 2025/9/18
// implementing tessaract's hypercuboid algorithm

#ifndef HYPERCUBOID_H
#define HYPERCUBOID_H

#include <map>
#include <set>
#include <vector>
#include <variant>
#include <optional>
#include <string>
#include <memory>
#include <functional>
#include "geometry.h"
#include "state.h"
#include "vec4.h"

struct physical_move
{
    full_move m;
};
struct arriving_move
{
    full_move m;
    std::shared_ptr<board> b;
    int idx; // index of corresponding departing move
    // not storing the axis of departing move because it can be found by `line_to_axis[m.from.l()]`
};
struct departing_move
{
    vec4 from;
    std::shared_ptr<board> b;
};
struct null_move
{
    vec4 tl;
};

//AxisLoc
using AL = std::variant<physical_move, arriving_move, departing_move, null_move>;
using moveseq = std::vector<full_move>;



class HC_search
{
    // local variables
    const state s;
    const std::map<int, int> line_to_axis; // map from timeline index to axis index
    const std::vector<std::vector<AL>> axis_coords; // axis_coords[i] is the set of all moves on i-th playable board
    const HC universe;
    const int sign; // sign for the new lines
    const int new_axis, dimension; // axes 0, 1, ..., new_axis-1 are playable lines
    // whereas new_axis, new_axis+1, ..., dimension-1 are the possible branching lines
    // identity: num_axes = universe.axes.size() = axis_coords.size()
    
    static HC_search build_HC(const state& s);
    
    std::optional<point> take_point(const HC& hc) const;
    // find_problem functions essentially just find the problem of p
    // but it also try to remove more points in the sub-hypercuboid that contains p
    std::optional<slice> find_problem(const point& p, const HC& hc) const;
    std::optional<slice> jump_order_consistent(const point& p, const HC& hc) const;
    std::optional<slice> test_present(const point& p, const HC& hc) const;
    std::optional<slice> find_checks(const point& p, const HC& hc) const;
    moveseq to_action(const point& p) const;
    
    //private aggregate constructor
    HC_search(state s, std::map<int, int> lm, std::vector<std::vector<AL>> crds, HC uni, int sg, int ax, int dim)
        : s(std::move(s)), line_to_axis(std::move(lm)), axis_coords(std::move(crds)), universe(std::move(uni)), sign(sg), new_axis(ax), dimension(dim) {}

public:
    // public constructor
    HC_search(const state& s) : HC_search(build_HC(s)) {}
    class iterator
    {
        const HC_search *parent;
        search_space ss;
        // iterator variables
        moveseq current_result;
        bool finished;
        void advance();
    public:
        iterator(const HC_search *parent, bool finished);
        iterator operator++();
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
        const moveseq& operator*();
    };
    iterator begin();
    iterator end();
};

#endif /* HYPERCUBOID_H */
