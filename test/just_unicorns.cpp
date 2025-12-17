#include "game.h"
#include <tuple>
#include <ranges>

std::string just_unicorns =
R"(
[Board "Custom - Odd"]
[Mode "5D"]
[Size "5x5"]
[1u1uk*/5/5/5/K*U1U1:0:1:w]

1. Kb2 / Kd4
2. Kc1 / Kc5
)";
int main()
{
    game g = game::from_pgn(just_unicorns);
    state s = g.get_current_state();
    std::cout << s.to_string();
    std::cout << s.get_piece(vec4(1, 0, 3, 0), false) << "\n";
    for(auto q : s.gen_piece_move(vec4(1, 0, 3, 0), false))
    {
        std::cout << q << "\n";
    }
////    ext_move m(vec4(0,1,1,0), vec4(0,2,1,0));
////    g.apply_move(m);
////    g.submit();
//    full_move fm0("(0T2)Rb4b1"), fm1("(1T2)Rc4c2");
//    ext_move em0(fm0), em1(fm1);
//    action a = action::from_vector({em0, em1}, g.get_current_state());
//    action b = action::from_vector({em1, em0}, g.get_current_state());
//    std::cout << a << "\n";
//    std::cout << b << "\n";
//    g.apply_move(em0);
//    g.apply_move(em1);
//    g.submit();
//    g.visit_parent();
//    g.apply_move(em1);
//    g.apply_move(em0);
//    g.submit();
//    std::cout << g.show_pgn() << "\n";
//    g.suggest_action();
//    g.suggest_action();
//    bool flag = g.suggest_action();
//    std::cout << flag << "\n";
//    for(auto & [act, txt] : g.get_child_moves())
//    {
//        std::cout << txt << " i.e. ";
//        std::cout << g.get_current_state().pretty_action(act) << "\n";
//    }
    std::cout << g.show_pgn() << "\n";
    return 0;
}

