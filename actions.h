#ifndef ACTIONS_H
#define ACTIONS_H

#include <variant>
#include <tuple>
#include <set>
#include <ostream>
#include "vec4.h"

/*
 A move is either moving a piece from coordinate p to coordinate q or the action of submition.
 In this implementation, I use `full_move` instead of `move` to avoid confusion with `std::move`.
 */
struct full_move {
    std::variant<std::monostate, std::tuple<vec4, vec4>> data;

    explicit full_move();
    explicit full_move(const vec4& p, const vec4& d);

    full_move(std::string);

    static full_move submit();
    static full_move move(const vec4& p, const vec4& d);

    bool nonempty();

    bool operator<(const full_move& other) const;
    bool operator==(const full_move& other) const;
    
    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const full_move& m);
};


/*
 An action (moveset) is a tree-shaped structure. The root is always temp_move::submit(). Each child is either a leaf move or a move followed by set of moves depending on it.
 */
struct actions {
    std::variant<full_move, std::tuple<full_move, std::set<actions>>> value;

    explicit actions(full_move m);
    explicit actions(full_move m, std::set<actions> s);

    static actions leaf(full_move m);
    static actions branch(full_move m, std::set<actions> s);

    bool operator<(const actions& other) const;
    bool operator==(const actions& other) const;

    friend std::ostream& operator<<(std::ostream& os, const actions& a);

private:
    std::ostream& pretty_print(std::ostream& os, const std::string& prefix, const int& now) const;
};

#endif // ACTIONS_H
