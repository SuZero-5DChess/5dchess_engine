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
#include "geometry.h"
#include "state.h"
#include "vec4.h"

struct physical_move
{
    vec4 from, to;
};
struct arriving_move
{
    vec4 to;
    board bb;
};
struct departing_move
{
    vec4 from;
    board bb;
};
struct null_move
{
    vec4 tl;
};

//AxisLoc
using AL = std::variant<physical_move, arriving_move, departing_move, null_move>;

class HC_search
{
    std::vector<std::map<int, AL>> axes;
    state s;
    HC universe;
public:
    HC_search(state s);
    class iterator
    {
        const HC_search* hc;
        std::vector<int> indices;
        bool end;
        std::optional<actions> current;
        void advance();
        bool finished;
    public:
        iterator(const HC_search* hc, bool end);
        iterator operator++();
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
        const actions& operator*() const;
        const actions* operator->() const;
    };
    iterator begin() const;
    iterator end() const;
};

#endif /* HYPERCUBOID_H */
