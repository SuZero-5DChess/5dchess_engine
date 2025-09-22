#ifndef ACTIONS_H
#define ACTIONS_H

#include <variant>
#include <tuple>
#include <set>
#include <ostream>
#include "vec4.h"

struct full_move
{
    vec4 from, to;
    full_move(vec4 from, vec4 to) : from(from), to(to) {}
    full_move(std::string);
    std::string to_string() const;
    bool operator<(const full_move& other) const;
    bool operator==(const full_move& other) const;
    friend std::ostream& operator<<(std::ostream& os, const full_move& fm);
};

/*
 A move is either moving a piece from coordinate p to coordinate q or the action of submition.
 In this implementation, I use `full_move` instead of `move` to avoid confusion with `std::move`.
 */
struct move5d {
    std::variant<std::monostate, full_move> data;

    explicit move5d();
    explicit move5d(const vec4& p, const vec4& d);

    move5d(std::string);

    static move5d submit();
    static move5d move(const vec4& p, const vec4& d);

    bool nonempty();

    bool operator<(const move5d& other) const;
    bool operator==(const move5d& other) const;
    
    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const move5d& m);
};


/*
 An action (moveset) is a tree-shaped structure. The root is always temp_move::submit(). Each child is either a leaf move or a move followed by set of moves depending on it.
 */
struct actions {
    std::variant<move5d, std::tuple<move5d, std::set<actions>>> value;

    explicit actions(move5d m);
    explicit actions(move5d m, std::set<actions> s);

    static actions leaf(move5d m);
    static actions branch(move5d m, std::set<actions> s);

    bool operator<(const actions& other) const;
    bool operator==(const actions& other) const;

    friend std::ostream& operator<<(std::ostream& os, const actions& a);

private:
    std::ostream& pretty_print(std::ostream& os, const std::string& prefix, const int& now) const;
};

#endif // ACTIONS_H
