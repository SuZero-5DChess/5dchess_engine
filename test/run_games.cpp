#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <regex>
#include "game.h"
#include "utils.h"


std::string t0_fen = ""
"[Size 8x8]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]\n"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]\n";


std::vector<full_move> pgn_to_moves(const std::string& input)
{
    std::string output;
    
    // Regex to match slashes ("/") and move numbers (e.g., "1.", "2.")
    std::regex pattern(R"((\d+\.)|(/))");

    // Create a stringstream from the input string
    std::istringstream stream(input);
    std::string line;

    while (std::getline(stream, line)) {
        // Replace matches with "submit"
        line = std::regex_replace(line, pattern, " submit ");
        output += line + "\n"; // Append to output with newline preserved
    }

    // Split the result by whitespace into a vector of moves
    std::vector<full_move> result;
    std::istringstream result_stream(output);
    std::string word;
    //remove the first "submit"
    result_stream >> word;
    while (result_stream >> word)
    {
        if(word.compare("submit") != 0)
        {
            result.push_back(full_move(word));
        }
        else
        {
            result.push_back(full_move::submit());
        }
    }
    return result;
}

void test1()
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
    std::vector<full_move> mvs = {
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
        std::cout << " ... success\n";
    }
//
//     g.undo();
//     g.undo();
//     g.undo();
//     g.redo();
//     std::cout << g.apply_move(full_move("(0T2)Rh8g8")) << endl;
    std::cout << g.get_current_state().m.to_string();
    std::cout << "test1 finished" << std::endl;
}


multiverse m0(t0_fen);

void run_game(std::string pgn)
{
    state s(m0);
    
    std::vector<full_move> mvs = pgn_to_moves(pgn);
    for(full_move mv : mvs)
    {
        bool flag = s.apply_move(mv);
        if(!flag)
        {
            std::cerr << "In run_game with this pgn:\n" << pgn << "\n";
            std::cerr << "failed to apply: " << mv << "\n";
            std::cerr << "current boards:\n";
            std::cerr << s.m.to_string() << std::endl;
            throw std::runtime_error("");
            break;
        }
    }
}

int main()
{
    std::vector<std::string> pgns = {R"(
1.(0T1)Ng1f3 / (0T1)Ng8f6 
2.(0T2)d2d4 / (0T2)d7d5 
3.(0T3)Bc1d2 / (0T3)c7c6 
4.(0T4)c2c4 / (0T4)Bc8f5 
5.(0T5)e2e3 / (0T5)e7e6 
6.(0T6)Nb1c3 / (0T6)Bf8b4 
7.(0T7)Qd1b3 / (0T7)Bb4c3 
8.(0T8)Bd2c3 / (0T8)Qd8b6 
9.(0T9)Qb3a3 / (0T9)Qb6c7 
10.(0T10)Bc3a5 / (0T10)b7b6 
11.(0T11)Nf3g5 / (0T11)b6a5 
12.(0T12)Ng5e6 / (0T12)f7e6 
13.(0T13)Qa3>>(0T9)e7 / (0T13)Qc7>>(0T8)h2 
14.(-1T9)Rh1h2 / (-1T9)Qd8c7 (1T9)Ke8e7 
15.(1T10)Nf3e5 (-1T10)Qb3b7 / (-1T10)Qc7>(1T10)e5 
16.(-1T11)Qb7c8 (1T11)Bc3b4 / (1T11)Ke7>>(1T10)d6 
17.(-2T11)Bc3b4
    )",
        R"(
1.(0T1)Ng1f3 / (0T1)Ng8f6 
2.(0T2)e2e3 / (0T2)d7d5 
3.(0T3)b2b3 / (0T3)c7c6 
4.(0T4)Bc1b2 / (0T4)e7e6 
5.(0T5)Bb2f6 / (0T5)Qd8f6 
6.(0T6)d2d4 / (0T6)Bf8b4 
7.(0T7)c2c3 / (0T7)Qf6e7 
8.(0T8)Qd1d2 / (0T8)Bb4d6 
9.(0T9)Bf1d3 / (0T9)Nb8d7 
10.(0T10)Qd2c2 / (0T10)Nd7f6 
11.(0T11)h2h3 / (0T11)a7a5 
12.(0T12)Nf3g5 / (0T12)g7g6 
13.(0T13)Qc2e2 / (0T13)Bc8d7 
14.(0T14)a2a4 / (0T14)e6e5 
15.(0T15)Ra1a2 / (0T15)e5d4 
16.(0T16)e3d4 / (0T16)Bd7e6 
17.(0T17)Ke1d1 / (0T17)Bd6f4 
18.(0T18)Ng5>>(0T17)g7 / (1T17)Ke8d7 
19.(1T18)Bd3b5 / (1T18)Ra8c8 (0T18)Qe7f8 
20.(1T19)Bb5a6 (0T19)Bd3b5 / (0T19)Nf6d7 (1T19)b7a6
)"
    };
    for(auto pgn : pgns)
        run_game(pgn);
    std::cout << "---= run_games.cpp: all tests passed =---" <<std::endl;
    return 0;
}
