#include <iostream>
#include <string>
#include <regex>

#include "hypercuboid.h"
#include "pgnparser.h"

std::string pgn =
R"(
[Size "4x4"]
[Board "Custom"]
[Mode "5D"]
[nbrk/3p*/P*3/KRBN:0:1:w]
1. Rb4 / Rxb4
2. N>>d3 / (L1)Bc3+
3. Nxc3
)";
//std::string pgn = R"(
//[Mode "5D"]
//[Board "Standard"]
//1. e3 / Nf6
//2w. Bb5 {Beware!}
//(2b. d5 {The right response})
//2b. c6
//3. c3 / cxb5
//4. Qb3 / Qa5
//5. Q>>xf7+~ (~T1) (>L1) {f7-sacrifice!} / (1T1)Kxf7
//6. (1T2)Nh3 / (1T2)e6
//7. (1T3)e3 / (1T3)Qf6
//8. (1T4)Qh5*
//)";


int main()
{
    pgnparser_ast::game g = *pgnparser(pgn).parse_game();
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
