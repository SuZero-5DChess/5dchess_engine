#ifndef STATE_H
#define STATE_H

#include "multiverse.h"
#include "actions.h"
#include <string>

struct state {
    multiverse m;
    /*
     `number_activated` is max(abs(white's activated lines), abs(black's activated lines)).
     `present` is in L,T coordinate (i.e. not u,v corrdinated).
     These numbers can be inherited from copy-construction; thus they are not necessarily equal to `m.get_present()`.
    */
    int number_activated, present, player;

    state(multiverse mtv);

    int new_line() const;

    bool can_submit() const;

    bool apply_move(full_move fm);
    bool apply_move(std::string s)
    {
        return apply_move(full_move(s));
    }

};

#endif //STATE_H
