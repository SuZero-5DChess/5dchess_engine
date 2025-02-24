#include <iostream>
#include "state.h"
#include "utils.h"
#include <vector>
#include <variant>

std::string t0_fen = ""
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]\n"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]\n";

 using namespace std;

 int main()
 {
     multiverse m(t0_fen);
     state st(m);
     full_move mv("(0T1)Ng1f3");
     
     cout << mv << endl;
//     
//     SHOW(vec4(-1,-1,-2,-1))
//     SHOW(vec4(1,1,1,1))
//     SHOW(vec4(-1,-1,-2,-1)+vec4(1,1,1,1))
     
     std::visit(overloads{
         [](std::monostate){},
         [&](std::tuple<vec4, vec4> data)
         {
             auto [p, d] = data;
             vector<vec4> deltas = m.gen_piece_move(p, st.player);
             print_range("deltas:", deltas);
//             SHOW(p)
//             SHOW(-vec4(5,2,1,0))
//             SHOW(-p)
//             SHOW(d)
         }
     }, mv.data);
     
     
     std::cout << (st.apply_move(mv) ? "true" : "false") << std::endl;
     
     std::cout << st.m.to_string();
     
     std::cout << "shutting down" << std::endl;
     
     return 0;
 }
