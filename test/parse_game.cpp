#include <iostream>
#include <vector>
#include "pgnparser.h"
#include "utils.h"

void test_actions()
{
    pgnparser("1.{} {{ab {c}}d}\n / (2.{1987yhwqsi}))").test_lexer();
    std::cout << "void\n";
    std::cout << pgnparser("$(L+7T-6)").parse_relative_board() << "\n";
    std::cout << pgnparser("(L+7T6)").parse_absolute_board() << "\n";
    std::cout << pgnparser("(T5)").parse_absolute_board() << "\n";
    std::cout << pgnparser("(L+5T7)").parse_absolute_board() << "\n";
    std::cout << pgnparser("(2. Kf7)", false).parse_absolute_board() << "\n";
    std::cout << pgnparser("(16b.Qxg5 17.", false).parse_absolute_board() << "\n";
    std::cout << pgnparser("16b.Qxg5 17.", false).parse_actions() << "\n";
    std::cout << pgnparser(R"(16b.(-2)Kxc3{}(-1)Qxg5    (0T9)f3 {bloody}{gibberish} (1T7)e2>>$(T-1)b1+~??? {%command1%}
{new-line {nested}}
 
 17.)", false).parse_actions() << "\n";
    std::cout << pgnparser("16b. R5g6 c3(17.Ke5)", false).parse_actions() << "\n";
    
    std::cout << pgnparser("(L+7T6)Q>>a3+~?!!(~T5)(>L-10)").parse_move() << "\n";
//    std::cout << pgnparser("16b. R5g6 c3>>(17.Ke5)", false).parse_actions() << "\n"; //error
//    std::cout << pgnparser("c3>>(17.Ke5)", false).parse_move() << "\n"; //error
}

void test_gametree()
{
    std::string str=R"(1w.e3 ( 1b. e6 {f7-sac}2.f3 / Nf6 3.Bb5 / Pc6
)(1b. c6 {branch2}) 1b.Nf6 {main} 2.Nf3 / d5 3.c4 / c6 4.cxd5/Qxd5
)";
    //std::string str = "1w.e3 (1b.e6 2.f3/Nf6) (1b. c6 {falsch}) 1b.Nf6";
    std::cout << pgnparser(str).parse_gametree() << "\n";
//    std::cout << pgnparser("1w. e3 (1b.e6) 1b.Nf6 {main} 2.Nf3 / d5 3.c4 / c6 4.cxd5/Qxd5", true, {1,false}).parse_gametree() << "\n";
}

void test_game()
{
    std::string str = R"(
[Mode "5D"]
[Board "Custom"]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:+0:0:b]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:-0:0:b]
1.e3 {/ e6
2.Nf3 / Nf6}
)";
    std::cout << pgnparser(str).parse_game() << std::endl;
}

int main()
{
    //test_actions();
    //test_gametree();
    test_game();
    std::cout << "---= parse_game.cpp: all tests passed =---" <<std::endl;
    return 0;
}

