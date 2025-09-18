#ifndef STATE_H
#define STATE_H

#include "multiverse.h"
#include "actions.h"
#include <string>
#include <set>
#include <optional>
#include <list>
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

    state(multiverse mtv);

    int new_line() const;

    bool can_submit() const;

    /*
     can_apply: Check if the move can be applied to the current state. If yes, return the new state after applying the move; otherwise return std::nullopt.
     Note that this function is different from `apply_move` in that it does not change the current state as a side effect.
    */
    std::optional<state> can_apply(full_move fm);
    
    /*
     apply_move: Apply move to the current state as a side effect. Return true if it is successfull.
     Parameter `UNSAFE=true`: unsafe mode, does not check whether the pending move is pseudolegal. If it is indeed not pseudolegal, the outcome may be unexpected.
     */
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
    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> get_timeline_status(int present_t, int present_c) const;
    
    /*
     find_check()
     For the 'player' and 'present' that is told from the shape of the board (which might be newer than the states's present), test if that player is able to capture an enermy royal piece.
     */
    bool find_check() const;
    
    /*
     find_check_impl<C>(lines)
     For all boards on the end of timelines specified in `lines` with color `C`,
     test if one of piece on that board with color `C` can capture an enermy royal piece.
     */
    template<bool C>
    bool find_check_impl(const std::list<int>& lines) const;
    
    std::map<vec4, bitboard_t> gen_movable_pieces() const;
    template<bool C>
    std::map<vec4, bitboard_t> gen_movable_pieces_impl(const std::vector<int>& lines) const;
    
    std::vector<std::pair<vec4,vec4>> find_all_checks() const;
    template<bool C>
    std::vector<std::pair<vec4,vec4>> find_all_checks_impl(const std::list<int>& lines) const;
};

std::ostream& operator<<(std::ostream& os, const match_status_t& status);

#endif //STATE_H
