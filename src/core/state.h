#ifndef STATE_H
#define STATE_H

#include "multiverse.h"
#include "actions.h"
#include <memory>
#include <string>
#include <set>
#include <optional>
#include <list>
#include <iostream>

class state
{
    std::unique_ptr<multiverse> m;
public:
    /*
     `present` is in L,T coordinate (i.e. not u,v corrdinated).
     These numbers can be inherited from copy-construction; thus they are not necessarily equal to `m.get_present()`.
    */
    int present, player;
    
	//match_status_t match_status;

    state(multiverse &mtv);
    
    // standard copy-constructors
    state(const state& other)
        : m(other.m->clone()) {}
    state(state&&) noexcept = default;
    state& operator=(state other) noexcept {
        swap(*this, other);
        return *this;
    }
    friend void swap(state& a, state& b) noexcept {
        std::swap(a.m, b.m);
    }

    int new_line() const;

    /*
     can_apply: Check if the move can be applied to the current state. If yes, return the new state after applying the move; otherwise return std::nullopt.
     Note that this function is different from `apply_move` in that it does not change the current state as a side effect.
    */
    std::optional<state> can_apply(full_move fm) const;
    std::optional<state> can_submit() const;
    
    /*
     apply_move: Apply move to the current state as a side effect. Return true if it is successfull.
     Parameter `UNSAFE=true`: unsafe mode, does not check whether the pending move is pseudolegal. If it is indeed not pseudolegal, the outcome may be unexpected.
     */
    template<bool UNSAFE = false>
    bool apply_move(full_move fm);
    template<bool UNSAFE = false>
    bool submit();
    
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
    std::map<vec4, bitboard_t> get_movable_pieces(std::vector<int> lines) const;
    template<bool C>
    std::map<vec4, bitboard_t> gen_movable_pieces_impl(const std::vector<int>& lines) const;
    
    std::vector<full_move>find_all_checks() const;
    template<bool C>
    std::vector<full_move> find_all_checks_impl(const std::list<int>& lines) const;

    // wrappers for low-level functions
    std::pair<int, int> apparent_present() const;
    std::pair<int, int> get_lines_range() const;
    std::pair<int, int> get_active_range() const;
    std::pair<int, int> get_timeline_start(int l) const;
    std::pair<int, int> get_timeline_end(int l) const;
    piece_t get_piece(vec4 p, int color) const;
    std::shared_ptr<board> get_board(int l, int t, int c) const;
    std::vector<std::tuple<int,int,int,std::string>> get_boards() const;
    generator<vec4> gen_piece_move(vec4 p) const;
    std::string to_string() const;
};

//std::ostream& operator<<(std::ostream& os, const match_status_t& status);

#endif //STATE_H
