#include "game.h"
#include <tuple>
#include <ranges>

std::string very_small_open =
//R"(
//[Size "4x4"]
//[Board "custom"]
//[Mode "5D"]
//[nbrk/3p*/P*3/KRBN:0:1:w]
//
//1. Bb2 / Nxb2
//2. Nxb2 / Kc3
//3. a4 / d1
//4. (0T4)Qa4>>(0T3)b3 / (0T4)Qd1>>x(0T2)b1
//5. (-1T3)Kxb1 / (-1T3)K>>(0T2)c3
//6. (-2T3)R>>(-1T3)b1 / (2T3)B>x(1T3)b3 (-2T3)d1
//7. (-1T4)Kb1>>x(-1T3)b1 / (3T3)d1
//8. (3T4)Nxd1 (1T4)N>(2T4)d2 (-2T4)Nxd1 /
//   (3T4)Rc1 (1T4)K>x(2T4)d2 (-1T4)Rc2 (-2T4)Kc3d3
//
//)";
R"(
{exiledKings}
[Mode "5D"]
[Board "Standard - Turn Zero"]
1.(0T1)Ng1f3 / (0T1)e7e6 
2.(0T2)b2b3 / (0T2)c7c6 
3.(0T3)e2e3 / (0T3)Qd8b6 
4.(0T4)Nf3g5 / (0T4)Qb6>>(0T0)f2
5.(-1T1)Ke1f2 / (-1T1)Ng8f6 
6.(-1T2)e2e3 / (-1T2)Nf6>>(-1T1)f4 
7.(-1T3)Kf2e1 / (-1T3)Ke8>>(0T4)d8 
8.(-1T4)Qd1f3 / (-1T4)f7f6 
9.(0T5)Qd1f3 (-1T5)Ng1h3 / (0T5)Ke8>>(0T4)d8 (-1T5)f6>>(0T5)f6
10.(-1T6)Qf3f7 (0T6)Qf3f7 / (0T6)Bf8>(-1T6)f7
)";
int main()
{
    game g(very_small_open);
    //g.apply_move(move5d("(0T1)d3d1"));
    std::cout << g.get_current_state().to_string();
//    print_range("Checking moves: ", g.get_current_state().find_checks());
//    std::cout << "Checking moves: ";
//    auto [t, c] = g.get_current_present();
//    for(full_move fm : g.get_current_state().find_checks())
//    {
//        std::cout << g.get_current_state().pretty_move(fm, 1-c) << " ";
//        std::cout << static_cast<int>(g.get_current_state().get_piece(fm.to, 1-c));
//    }
//    std::cout << std::endl;
//    g.apply_move(move5d::submit());
//    auto mv = g.get_current_state().parse_move("a3");
//    std::cout << mv << std::endl;
    return 0;
}
