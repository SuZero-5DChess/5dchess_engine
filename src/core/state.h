#ifndef STATE_H
#define STATE_H

#include "multiverse.h"
#include "actions.h"
#include <string>
#include <set>
#include <optional>
#include <iostream>

struct state
{
    multiverse m;
    /*
     `present` is in L,T coordinate (i.e. not u,v corrdinated).
     These numbers can be inherited from copy-construction; thus they are not necessarily equal to `m.get_present()`.
    */
    int present, player;

	match_status_t match_status;
    //mutable std::set<vec4> critical_coords;

    state(multiverse mtv);
    // state(const state& other);
    // state& operator=(const state& other);

    // void clear_cache();

    int new_line() const;

    bool can_submit() const;

    template<bool UNSAFE = false>
    bool apply_move(full_move fm);
    

    /*
     get_timeline_status() returns `std::make_tuple(mandatory_timelines, optional_timelines, unplayable_timelines)`
     where:
     mandatory_timelines are the timelines that current player must make a move on it
     optional_timelines are the timelines that current player can choose to play or not
     unplayable_timelines are the timelines that current player can't place a move on
     */
    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> get_timeline_status() const;
    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> get_timeline_status(int present_t, int present_v) const;
    
    struct mobility_data
    {
        /*
         if `is_check` is true, then `critical_coords` contains (usually just one) friendly piece that checks the opponents king
         otherwise, `critical_coords` contains all friendly pieces that are able to perform at least one pseudolegal move
         */
        bool is_check;
        std::set<vec4> critical_coords;
    };
    
    mobility_data find_check();

    template<bool C>
    mobility_data find_check_impl() const;
};

std::ostream& operator<<(std::ostream& os, const match_status_t& status);

#endif //STATE_H
