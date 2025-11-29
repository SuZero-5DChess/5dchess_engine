#include <iostream>
#include <string>
#include <regex>
#include <chrono>

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
{ctp1}{checkmate}
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
std::string pgn5 = R"(
{NP}{8 options}
[Board "Standard"]
[Mode "5D"]

1. h4 / h5
2. Rh3 / Rh6
3. Ra3 / Rg6
4. Rxa7 / Rxg2
5. Rxb7 / Rxg1
6. Rxb8 / Rg2
7. Rxc8 / Rxf2
8. Bg2 / Rxg2
9. Rxc7 / Rxe2
10. Kf1 / Rxd2
11. Rxd7 / Qc7
12. Rxc7 / Rxd1
13. Kg2 / Rxc1
14. Rxe7 / Kd8
15. Rxf7 / Rxb1
16. Rxf8 / Kc7
17. Rxg8 / Rxb2
18. Rxg7 / Kd6
19. Rh7 / Rbxa2
20. Rxh5 / Rxc2
21. Kh1 / Rc4
22. Kg1 / Rxh4
23. (0T23)Kg1>>(0T22)g1~ (>L1) / (0T23)Ke6 (1T22)Rxh4
24. (1T23)Khg2 / (1T23)Rh3
25. (0T24)Rg5 (1T24)Rg5 / (0T24)Kf6 (1T24)Rh3>>(0T24)h3~ (>L-1)
26. (0T25)Rgg1 (1T25)Rg3 (-1T25)Rgg1 / (-1T25)Kd5 (0T25)Rh5 (1T25)Ke5
27. (-1T26)Rg1>>(-1T25)g1~ (>L2) / (2T25)Kf6
28. (0T26)Rge1 (2T26)R5g2 (1T26)Rga3 / (-1T26)Rh5 (2T26)Rhh8 (1T26)Kf6 (0T26)Kg7
29. (2T27)Rga2 (1T27)K1h1 (-1T27)Ra2 (0T27)Reb1 / (-1T27)Rh5>>(-1T25)h5~ (>L-2)
30. (-1T28)Ra1 (-2T26)Ra2 / (-1T28)Rc3 (-2T26)Kd5
31. (-1T29)Rh1 (-2T27)Rag2 / (-1T29)Rc3>>(-1T25)c3~ (>L-3) (0T27)Rhh8
32. (-3T26)Raf1 / (-3T26)Kd5 (2T27)Ke5 (1T27)Kg7
33. (-3T27)Rf5 / (-3T27)Kc4 (-2T27)Kc4
34. (2T28)Rg1>(0T28)g1 (1T28)Rc3 (-2T28)Rg2>>(0T28)g2~ (>L3) (-3T28)Rgg5 / (0T28)Kh7 (3T28)Kh7 (2T28)R3h7 (1T28)Kh8 (-2T28)Rha5 (-3T28)Kb3
35. (1T29)Kh1>(2T29)g1 (-3T29)Rg3 (3T29)Rf2 (0T29)Rb2 (-2T29)Rg2 / (3T29)Kg7 (-2T29)Re4 (-3T29)Ka3 (1T29)Rb8 (0T29)Rhb8 (2T29)Rhb8
36. (-3T30)Rf5>(-1T30)f5 (-2T30)Rg1 (3T30)Ra5 (1T30)Rc3g3 (2T30)Ra2c2 (0T30)Rf2 /   (3T30)Rh8b8 (2T30)Ke5e6 (1T30)Rb8a8 (0T30)Rb8c8 (-1T30)Kd5e6 (-2T30)Re4e5 (-3T30)Ka3b2
37. (3T31)Ra5e5 (2T31)Rc2c3 (1T31)Rg3f3 (0T31)Rg1f1 (-1T31)Rf5a5 (-2T31)Rg1g2 (-3T31)Rg3g4 / (3T31)Ra8a1 (2T31)Rh7a7 (1T31)Ra8b8 (0T31)Kh7g8 (-1T31)Ke6f7 (-2T31)Ra5a1 (-3T31)Rh4h8
38. (3T32)Re5e3 (2T32)Ra1a2 (1T32)Kg2f2 (0T32)Ra1e1 (-1T32)Rh1f1 (-2T32)Rg2g1 (-3T32)Rg4g1 / (3T32)Ra1>>(3T30)a1 (2T32)Rb8c8 (1T32)Rb8a8 (0T32)Rc8c3 (-1T32)Kf7g8 (-2T32)Rh3h8 (-3T32)Rh3h7
39. (3T33)Rb1g1 (2T33)Ra2a6 (1T33)Rf3h3 (0T33)Rf2g2 (-1T33)Ra5g5 (-2T33)Rg1g4 (-3T33)Rg1g2 (-4T31)Ra5e5 / (2T33)Ke5 (1T33)Kg8  (-2T33)Kc3 (-3T33)Ka1 (-4T31)Rh8b8
40.  (2T34)Ra6h6 (1T34)Rh3h1   (-2T34)Rg4h4 (-3T34)Rg2h2 (-4T32)Rb1b3 / (2T34)Ra8b8 (1T34)Ra8b8 (-2T34)Ra8b8 (-3T34)Ra8b8 (-4T32)Kg7g8
41. (-4T33)Rb3g3 / (3T33)Kh8 (0T33)Kh8 (-1T33)Kh8 (-4T33)Kh8
42. (3T34)Rf2h2 (0T34)Rf1h1 (-1T34)Rf1h1 (-4T34)Rf2h2
)";
std::string pgn6=R"(
[Mode "5D"]
[Board "Standard"]

1. (0T1)Nc3 / (0T1)a6
2. (0T2)Rb1 / (0T2)a5
3. (0T3)Rb1>>(0T2)b1~ / (1T2)a5 (0T3)Nb8>>(0T2)b6~
{4. (0T4)Nc3>>(0T2)d3~ (1T3)Ra1>>(0T3)a1}
)";

void search_all(std::string pgn)
{
    pgnparser_ast::game g = *pgnparser(pgn).parse_game();
    state s(g);
    std::cout << s.to_string() << std::endl;
    std::cout << "starting_test:\n";
    auto [w, ss] = HC_info::build_HC(s);
    std::vector<moveseq> legal_moves;
//    auto it = ss.hcs.end();
//    it--; it--;
//    HC hc = *it;
//    hc.axes[1] = {0};
//    std::cout << hc.to_string();
//    search_space ss1 {{hc}};

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
        std::cout << "\n----------------------------\n\n";
    }
    std::cout << "\n----------------------------\n\n";
    std::cout << "Summary: totally " << legal_moves.size() << " options\n";
    for(moveseq x:legal_moves)
    {
        for(full_move m : x)
        {
            std::cout << s.pretty_move<>(m) << " ";
        }
        std::cout << "\n";
    }
}

void count(std::string pgn)
{
    pgnparser_ast::game g = *pgnparser(pgn).parse_game();
    state s(g);
    auto [w, ss] = HC_info::build_HC(s);
    std::vector<moveseq> legal_moves;
    for(auto x : w.search(ss))
    {
        legal_moves.push_back(x);
    }
    std::cout << "Summary: totally " << legal_moves.size() << " options\n";
}


int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    count(pgn6);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "took " << duration << " ms\n";
    return 0;
}
