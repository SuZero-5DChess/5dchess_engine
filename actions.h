#ifndef ACTIONS_H
#define ACTIONS_H

#include <variant>
#include <tuple>
#include <set>
#include <ostream>
#include "vec4.h"

/*
 A move is either moving a piece from coordinate p to coordinate q or the action of submition.
 In this implementation, I use `temp_move` instead of `move` to avoid confusion with `std::move`.
 */
struct temp_move {
    std::variant<std::monostate, std::tuple<vec4, vec4>> data;

    explicit temp_move();
    explicit temp_move(const vec4& p, const vec4& d);

    temp_move(std::string);

    static temp_move submit();
    static temp_move move(const vec4& p, const vec4& d);

    bool operator<(const temp_move& other) const;
    bool operator==(const temp_move& other) const;

    friend std::ostream& operator<<(std::ostream& os, const temp_move& m);
};


/*
 An action (moveset) is a tree-shaped structure. The root is always temp_move::submit(). Each child is either a leaf move or a move followed by set of moves depending on it.
 */
struct actions {
    std::variant<temp_move, std::tuple<temp_move, std::set<actions>>> value;

    explicit actions(temp_move m);
    explicit actions(temp_move m, std::set<actions> s);

    static actions leaf(temp_move m);
    static actions branch(temp_move m, std::set<actions> s);

    bool operator<(const actions& other) const;
    bool operator==(const actions& other) const;

    friend std::ostream& operator<<(std::ostream& os, const actions& a);

private:
    std::ostream& pretty_print(std::ostream& os, const std::string& prefix, const int& now) const;
};

#endif // ACTIONS_H
