#include <iostream>
#include <string>
#include <regex>

#include "hypercuboid.h"
#include "pgnparser.h"

std::string pgn1 =
R"(
[Size "4x4"]
[Board "Custom"]
[Mode "5D"]
[nbrk/3p*/P*3/KRBN:0:1:w]
1. Rb4 / Rxb4
2. N>>d3 / (L1)Bc3+
3. Nxc3
)";
std::string pgn2 = R"(
[Mode "5D"]
[Board "Standard"]
1. e3 / Nf6
2w. Bb5 {Beware!}
(2b. d5 {The right response})
2b. c6
3. c3 / cxb5
4. Qb3 / Qa5
5. Q>>xf7+~ (~T1) (>L1) {f7-sacrifice!} / (1T1)Kxf7
6. (1T2)Nh3 / (1T2)e6
7. (1T3)e3 / (1T3)Qf6
8. (1T4)Qh5*
)";
std::string pgn3 = R"(
[Size "4x4"]
[Board "Custom"]
[Mode "5D"]
[nbrk/3p*/P*3/KRBN:0:1:w]
1. Bb2 / Nxb2
2. Nxb2 / Kc3
3. a4 / d1
4. (0T4)Qa4>>(0T3)b3 / (0T4)Qd1>>x(0T2)b1
5. (-1T3)Kxb1 / (-1T3)K>>(0T2)c3
6. (-2T3)R>>(-1T3)b1 / (2T3)B>x(1T3)b3 (-2T3)d1
7. (-1T4)Kb1>>x(-1T3)b1 / (3T3)d1
8. (3T4)Nxd1 (1T4)N>(2T4)d2 (-2T4)Nxd1 /
   (3T4)Rc1 (1T4)K>x(2T4)d2 (-1T4)Rc2 (-2T4)Kc3d3
)";
std::string pgn4 = R"(
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
    pgnparser_ast::game g = *pgnparser(pgn3).parse_game();
    state s(g);
    std::cout << s.to_string() << std::endl;
    std::cout << "starting_test:\n";
    auto [w, ss] = HC_info::build_HC(s);
    std::vector<moveseq> legal_moves;
    for(auto x : w.search(ss))
    {
        if(x.size() == 0)
        {
            std::cout << "No avilable action in this turn";
        }
        else
        {
            std::cout << "Valid action with " << x.size() << " moves: ";
            legal_moves.push_back(x);
            for(full_move m : x)
            {
                std::cout << s.pretty_move(m) << " ";
            }
        }
        std::cout << "\n";
        //break;
        std::cout << "\n----------------------------\n\n";
    }
    std::cout << "\n----------------------------\n\n";
    std::cout << "Summary: totally " << legal_moves.size() << " options\n";
    for(moveseq x:legal_moves)
    {
        for(full_move m : x)
        {
            std::cout << s.pretty_move(m) << " ";
        }
        std::cout << "\n";
    }
    return 0;
}
