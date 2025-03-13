#include <iostream>
#include "game.h"
#include "utils.h"
#include <vector>
#include <variant>

std::string t0_fen = ""
"[Size 8x8]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]\n"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]\n";

 using namespace std;

 int main()
 {
     game g(t0_fen);
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
//     
     vector<full_move> mvs = {
         full_move("(0T1)e2e3"),
         full_move::submit(),
         full_move("(0T1)g8f6"),
         full_move::submit(),
         full_move("(0T2)f1c4"),
         full_move::submit(),
         full_move("(0T2)g7g5"),
         full_move::submit(),
         full_move("(0T3)g1h3"),
         full_move::submit(),
         full_move("(0T3)g5g4"),
         full_move::submit(),
         full_move("(0T4)e1g1"),
         full_move::submit(),
     };
     
     for(full_move mv : mvs)
     {
         std::cout << "Applying move: " << mv;
         bool flag = g.apply_move(mv);
         if(!flag)
         {
             std::cout << " ... failure\n";
             break;
         }
         cout << " ... success\n";
     }
//     
//     g.undo();
//     g.undo();
//     g.undo();
//     g.redo();
//     std::cout << g.apply_move(full_move("(0T2)Rh8g8")) << endl;
     std::cout << g.get_current_state().m.to_string();
//     
//     SHOW(vec4(0,1,0,0))
//     SHOW(vec4(0,2,0,0))
//     auto fm = full_move::move(vec4(0,1,0,0),vec4(0,1,0,0));
//     std::cout << fm << endl;
//     std::cout << g.apply_move(fm) << endl;
     std::cout << "shutting down" << std::endl;
     return 0;
 }
