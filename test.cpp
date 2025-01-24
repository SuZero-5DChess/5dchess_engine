#include <stdio.h>
#include "multiverse.h"
#include <iostream>
#include "vec4.h"
#include <cstdio>

std::string t0_fen = ""
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/4P3/P*P*P*P*1P*P*P*/R*NBQK*BNR*:0:1:b]"
"[r*nbqk*b1r*/p*p*p*p*p*p*p*p*/8/8/8/4P3/P*P*P*P*1P*P*P*/R*NBQK*BNR*:0:2:w]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/6n1/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:1:w]"
;

using namespace std;

int main()
{
    multiverse m(t0_fen);
    /*
    for(int i = -10; i < 10; i++)
    { 
        for(int j = -10; j < 10; j++)
        {
            for(int c = 0; c < 2; c++)
            {
                vec4 v(1,1,i,j);
                if(m.inbound(v, c))
                {
                    cout << v << (c?'b':'w') << endl;
                }
            }
        }
    }
     */
    vec4 p(1, 0, 1, -1);
    cout << vec4(1,2,0,0).outbound() << endl;
    cout << piece_name(m.get_piece(p, 0)) << endl;
    vector<vec4> moves = m.gen_piece_move(p, 0);
    for(vec4 v : moves)
    {
        cout << p+v << static_cast<int>((m.get_piece(p+v, 0))) << endl;
    }
    //cout << m.to_string() << endl;
    std::cout << "shutting down" << std::endl;
    return 0;
}
