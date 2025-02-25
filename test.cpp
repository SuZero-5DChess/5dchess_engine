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
     
//     full_move mv("(0T1)Ng1f3");
//     std :: cout << st.m.get_present() << endl;
//     st.apply_move(mv);
//     std :: cout << st.m.get_present() << endl;
//     std :: cout << st.m.to_string() << endl;
     
//     cout << mv << endl;
//     
//     std::visit(overloads{
//         [](std::monostate){},
//         [&](std::tuple<vec4, vec4> data)
//         {
//             auto [p, d] = data;
//             vector<vec4> deltas = m.gen_piece_move(p, st.player);
//             print_range("deltas:", deltas);
//             SHOW(p)
//             SHOW(-vec4(5,2,1,0))
//             SHOW(-p)
//             SHOW(d)
//         }
//     }, mv.data);
     
     vector<full_move> mvs = {
         full_move("(0T1)e2e3"),
         full_move::submit(),
         full_move("(0T1)Ng8>>(0T0)g6"),
         full_move::submit(),
         full_move("(1T1)e2e4"),
         full_move::submit(),
         full_move("(1T1)Ng6e5"),
         full_move::submit(),
     };
     
     for(full_move mv : mvs)
     {
         std::cout << "Applying move: " << mv;
         bool flag = st.apply_move(mv);
         if(!flag)
         {
             std::cout << " ... failure\n";
             break;
         }
         cout << " ... success\n";
     }
     
     std::cout << st.m.to_string();
     
     std::cout << "shutting down" << std::endl;
     
     return 0;
 }
