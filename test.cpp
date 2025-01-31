
#include <iostream>
/*

#include <cstdio>
#include <stdio.h>

 */
#include "multiverse.h"
#include "utils.h"
#include <vector>
#include "vec4.h"

std::string t0_fen = ""
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]\n"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]\n"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/4P3/P*P*P*P*1P*P*P*/R*NBQK*BNR*:0:1:b]\n"
"[r*nbqk*b1r*/p*p*p*p*p*p*p*p*/8/8/8/4P3/P*P*P*P*1P*P*P*/R*NBQK*BNR*:0:2:w]\n"
"[r*nbqk*b1r*/p*p*p*p*p*p*p*p*/8/8/2B5/4P3/P*P*P*P*1P*P*P*/R*NBQK*1NR*:0:2:b]\n"
"[r*nbqk*b1r*/p*p*p*1p*p*p*p*/3p4/8/2B5/4P3/P*P*P*P*1P*P*P*/R*NBQK*1NR*:0:3:w]\n"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/6n1/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:1:w]\n"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/6n1/8/P7/8/1P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:1:b]\n"
"[r*nbqk*bnr*/p*1p*p*p*p*p*p*/6n1/1p6/P7/8/1P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:2:w]\n"
"[r*nbqk*bnr*/p*1p*p*p*p*p*p*/6n1/1P6/8/8/1P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:2:b]\n"
"[r*nbqk*bnr*/p*2p*p*p*p*p*/6n1/1Pp5/8/8/1P*P*P*P*P*P*P*/R*NBQK*BNR*:-1:3:w]\n";

using namespace std;

int main()
{
    multiverse m(t0_fen);
    vec4 q(1,2,5,0);
    bool b = m.inbound(q, 0);
    cout << b << endl;
    
    vec4 p(2, 3, 3, 0);
    
    for(int i = -2; i < 3; i++)
    {
        for(int j = -2; j < 3; j++)
        {
            int c = 0;
            vec4 v(i,j,0,0);
            cout << v << p+v << (c?'b':'w') << (m.inbound(p+v, c) ? "+" : "") << endl;
//            if(m.inbound(p+v, c))
//            {
//                cout << v << p+v << (c?'b':'w') << endl;
//            }
        
        }
    }
     
    
    //cout << vec4(1,2,0,0).outbound() << endl;
    //cout << piece_name(m.get_piece(p, 0)) << endl;
    vector<vec4> moves = m.gen_piece_move(p, 0);
    for(vec4 v : moves)
    {
        cout << p+v << static_cast<int>((m.get_piece(p+v, 0))) << endl;
    }
    //cout << m.to_string() << endl;
    std::cout << "shutting down" << std::endl;
    
//    std::tuple mytuple(1,std::string("Hello"),12.5);
//    std::cout<< mytuple << std::endl;
//    std::vector<std::pair<int,vec4>> xs = {std::make_pair(2,vec4(0, 0, 0, 1)), std::make_pair(4,vec4(1,1,1,2))};
//    print_range("es sei:", xs);
     
    return 0;
}
