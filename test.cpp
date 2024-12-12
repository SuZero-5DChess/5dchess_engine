#include <stdio.h>
#include "board_5d.h"
#include <iostream>

std::string t0_fen = ""
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/4P3/P*P*P*P*1P*P*P*/R*NBQK*BNR*:0:1:b]"
"[r*nbqk*b1r*/p*p*p*p*p*p*p*p*/8/8/8/4P3/P*P*P*P*1P*P*P*/R*NBQK*BNR*:0:2:w]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/6n1/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:1:w]"
;

int main()
{
    multiverse board(t0_fen);
    //std::cout << (int)KING_UW << " " << (int)'K' << " " <<  std::endl;
    const auto [l,t,c,s] = board.get_boards()[0];
    std::cout << l << "T" << t << (c?'b':'w') << ":" << s << std::endl;
    return 0;
}