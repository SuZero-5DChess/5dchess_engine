#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <regex>
#include <chrono>
#include "game.h"
#include "utils.h"


std::string t0_fen = ""
"[Size 8x8]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]\n"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]\n";


std::vector<full_move> pgn_to_moves_(const std::string& input)
{
    std::string output;
    
    // Regex to match slashes ("/") and move numbers (e.g., "1.", "2.")
    std::regex pattern(R"((\d+\.)|(/))");
    const static std::regex comment_pattern(R"(\{.*?\})");
    
    // Create a stringstream from the input string
    std::string clean_input = std::regex_replace(input, comment_pattern, "");
    std::istringstream stream(clean_input);
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

void run_game(std::vector<full_move> mvs)
{
    state s(m0);
    
//    std::cerr << "Running new game ..." << std::endl;
    
    for(full_move mv : mvs)
    {
//        std::cerr << "apply " << mv << std::endl;
        bool flag = s.apply_move(mv);
        if(!flag)
        {
            std::cerr << "In run_game:\n";
            std::cerr << "current boards:\n";
            std::cerr << s.m.to_string() << std::endl;
            std::cerr << "failed to apply: " << mv << "\n";
            std::visit(overloads {
                [&](std::monostate){},
                [&](std::tuple<vec4, vec4> data)
                {
                    auto [p, _] = data;
                    std::cerr << "The allowed moves are: ";
                    for(vec4 d : s.m.gen_piece_move(p, s.player))
                    {
                        std::cerr << full_move::move(p, d) << " ";
                    }
                    std::cerr << std::endl;
                }
            }, mv.data);
            exit(-1);
        }
    }
//    
//    std::cerr << "all moves are legal" << std::endl;
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
    )",  R"(
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
)", R"(
1.(0T1)Ng1f3/(0T1)Ng8f6
2.(0T2)d2d4/(0T2)d7d5
3.(0T3)Bc1d2/(0T3)c7c6
4.(0T4)e2e3/(0T4)Bc8g4
5.(0T5)c2c4/(0T5)e7e6
6.(0T6)c4d5/(0T6)Qd8d5
7.(0T7)Nb1c3/(0T7)Qd5d7
8.(0T8)g2g3/(0T8)Nb8a6
9.(0T9)Ra1c1/(0T9)Ra8c8
10.(0T10)Bf1g2/(0T10)Bf8d6
11.(0T11)Qd1e2/(0T11)b7b5
12.(0T12)Ke1g1/(0T12)b5b4
13.(0T13)Nc3a4/(0T13)c6c5
14.(0T14)Na4b6/(0T14)a7b6
15.(0T15)Qe2a6/(0T15)Bg4f3
16.(0T16)Bg2f3/(0T16)h7h5
17.(0T17)Bf3b7/(0T17)Rc8b8
18.(0T18)Bb7>>(0T16)d7/(1T16)Nf6d7
19.(1T17)Qa6c8/(0T18)Qd7>(1T17)c8
20.(1T18)Bd2b4/(1T18)Bf3g2
21.(1T19)Kg1g2 (0T19)Qa6b5/(0T19)Nf6d7 (1T19)Qc8c6
22.(0T20)Qb5>(1T20)c6/(0T20)Bd6>(1T20)c6
23.(1T21)f2f3 (0T21)d4c5/(0T21)b6c5 (1T21)c5b4
24.(1T22)Rc1c6 (0T22)b2b3/(1T22)Bd6e7 (0T22)Nd7f6
25.(1T23)Rc6c8 (0T23)Rc1c5/(0T23)Nf6g4 (1T23)Be7d8
26.(1T24)Kg2g1 (0T24)Rc5c4/(0T24)Ng4>>(0T23)g2
27.(-1T24)Kg1g2/(-1T24)Nf6g4 (1T24)Ke8e7
28.(0T25)Bd2>>(0T20)d7/(2T20)Ke8f8
29.(2T21)Bd7e6/(2T21)f7e6
30.(2T22)Qb5d7
)", R"(
1.{1:48}(0T1)e2e3 / {5:10}(0T1)Ng8f6 
2.{1:48}(0T2)Ng1f3 / {5:10}(0T2)e7e6 
3.{1:48}(0T3)c2c3 / {5:10}(0T3)c7c6 
4.{1:48}(0T4)d2d4 / {5:10}(0T4)d7d5 
5.{1:48}(0T5)a2a4 / {5:10}(0T5)a7a5 
6.{1:48}(0T6)Nb1a3 / {5:10}(0T6)Bf8d6 
7.{1:48}(0T7)Bc1d2 / {5:10}(0T7)Bc8d7 
8.{1:48}(0T8)g2g3 / {5:10}(0T8)Nf6e4 
9.{1:48}(0T9)Bf1g2 / {5:10}(0T9)Qd8f6 
10.{1:48}(0T10)Qd1e2 / {5:10}(0T10)h7h5 
11.{1:48}(0T11)c3c4 / {5:10}(0T11)h5h4 
12.{1:48}(0T12)g3g4 / {5:10}(0T12)h4h3 
13.{1:48}(0T13)Bg2f1 / {5:10}(0T13)Nb8a6 
14.{1:48}(0T14)c4d5 / {5:10}(0T14)Bd6b4 
15.{1:48}(0T15)Na3c4 / {5:10}(0T15)e6d5 
16.{1:48}(0T16)Nc4e5 / {5:10}(0T16)Bd7g4 
17.{1:48}(0T17)Ne5g4 / {5:10}(0T17)Qf6f3 
18.{1:48}(0T18)Bd2b4 / {5:10}(0T18)Qf3e2 
19.{1:48}(0T19)Bf1e2 / {5:10}(0T19)Na6b4 
20.{1:48}(0T20)Ke1g1 / {5:10}(0T20)f7f6 
21.{1:48}(0T21)f2f3 / {5:10}(0T21)Ne4g5 
22.{1:48}(0T22)e3e4 / {5:10}(0T22)Ng5>>(0T21)g3 
23.{1:48}(0T23)Ng4>>(0T21)f4 / {5:10}(1T21)Ne4g3 
24.{1:48}(1T22)f2g3 (-1T22)h2g3 / {5:10}(-1T22)h3h2 (1T22)Nb4c2 
25.{1:48}(-1T23)Ng4h2 (1T23)Nf4e6 / {5:10}(-1T23)Ne4>>(-1T22)e2 
26.{1:48}(-1T24)Be2>(-2T23)e2 
)",R"(1.(0T1)e2e3 / (0T1)Ng8f6 
2.(0T2)Ng1f3 / (0T2)e7e6 
3.(0T3)c2c3 / (0T3)c7c5 
4.(0T4)Qd1a4 / (0T4)b7b6 
5.(0T5)Qa4h4 / (0T5)Bc8b7 
6.(0T6)c3c4 / (0T6)Nb8c6 
7.(0T7)Nb1c3 / (0T7)Nc6b4 
8.(0T8)Bf1e2 / (0T8)Nb4c2 
9.(0T9)Ke1f1 / (0T9)Nc2a1 
10.(0T10)Be2d3 / (0T10)a7a6 
11.(0T11)b2b3 / (0T11)Qd8c7 
12.(0T12)Nc3e4 / (0T12)Ke8c8 
13.(0T13)Ne4f6 / (0T13)g7f6 
14.(0T14)Qh4f6 / (0T14)Rh8g8 
15.(0T15)Rh1g1 / (0T15)Bf8g7 
16.(0T16)Qf6f7 / (0T16)Rd8f8 
17.(0T17)Qf7h5 / (0T17)Bb7f3 
18.(0T18)g2f3 / (0T18)Qc7b7 
19.(0T19)Bd3h7 / (0T19)Bg7>>(0T14)g2 
20.(-1T15)Kf1g2 / (-1T15)Rh8g8 
21.(-1T16)Kg2f1 / (-1T16)Bf8g7 
22.(-1T17)Qf6g5 / (-1T17)Bb7f3 
23.(-1T18)Qg5d8 / (-1T18)Qc7d8 
24.(0T20)Qh5>>(0T18)f3 / (1T18)Rf8f3
)", R"(
1.{20:05}(0T1)Ng1f3 / {20:05}(0T1)Ng8f6 
2.{20:05}(0T2)d2d4 / {20:05}(0T2)d7d5 
3.{20:05}(0T3)c2c3 / {20:05}(0T3)c7c6 
4.{20:05}(0T4)Bc1g5 / {20:05}(0T4)Bc8f5 
5.{20:03}(0T5)e2e3 / {20:05}(0T5)e7e6 
6.{20:03}(0T6)Bf1d3 / {20:02}(0T6)Bf5d3 
7.{20:03}(0T7)Qd1d3 / {20:02}(0T7)Qd8c7 
8.{19:58}(0T8)Nb1d2 / {19:56}(0T8)Nb8d7 
9.{19:58}(0T9)h2h3 / {19:55}(0T9)Ke8c8 
10.{19:58}(0T10)Bg5f4 / {19:55}(0T10)Bf8d6 
11.{19:54}(0T11)Bf4d6 / {19:55}(0T11)Qc7d6 
12.{19:34}(0T12)Qd3c2 / {19:50}(0T12)Nd7b6 
13.{19:24}(0T13)Ke1c1 / {18:23}(0T13)Nb6c4 
14.{18:24}(0T14)Nd2c4 / {18:23}(0T14)d5c4 
15.{17:36}(0T15)g2g3 / {15:52}(0T15)b7b5 
16.{16:15}(0T16)Nf3e5 / {15:52}(0T16)Nf6d7 
17.{15:54}(0T17)Ne5f7 / {15:36}(0T17)Qd6c7 
18.{15:38}(0T18)a2a3 / {14:25}(0T18)Rd8f8 
19.{14:52}(0T19)Nf7h8 / {14:25}(0T19)Rf8h8 
20.{14:51}(0T20)Rd1d2 / {14:23}(0T20)g7g6 
21.{14:15}(0T21)Qc2b1 / {13:08}(0T21)Nd7b6 
22.{13:57}(0T22)Qb1a2 / {12:47}(0T22)Nb6a4 
23.{13:57}(0T23)Rd2c2 / {12:39}(0T23)Rh8f8 
24.{13:24}(0T24)Rh1h2 / {10:00}(0T24)Na4b6 
25.{13:12}(0T25)b2b3 / {8:21}(0T25)c4b3 
26.{13:12}(0T26)Qa2b3 / {8:21}(0T26)Nb6c4 
27.{12:57}(0T27)a3a4 / {6:51}(0T27)Qc7g3 
28.{11:04}(0T28)Rh2h1 / {6:51}(0T28)Rf8f2 
29.{10:33}(0T29)Rh1e1 / {5:41}(0T29)Qg3e3 
30.{9:40}(0T30)Re1e3 / {5:41}(0T30)Rf2f1 
31.{9:07}(0T31)Re3e1 / {5:39}(0T31)Rf1e1 
)", R"(
1.{20:05}(0T1)Ng1f3 / {20:05}(0T1)Ng8f6 
2.{20:05}(0T2)d2d4 / {19:44}(0T2)d7d5 
3.{20:05}(0T3)c2c3 / {19:44}(0T3)c7c6 
4.{20:05}(0T4)Bc1f4 / {19:36}(0T4)Bc8f5 
5.{20:05}(0T5)g2g4 / {18:04}(0T5)Bf5g4 
6.{20:05}(0T6)Nf3e5 / {17:38}(0T6)Bg4h5 
7.{20:05}(0T7)Bf1h3 / {16:47}(0T7)Nb8d7 
8.{19:43}(0T8)Ne5d7 / {16:41}(0T8)Nf6d7 
9.{19:40}(0T9)Qd1d3 / {16:22}(0T9)Nd7f6 
10.{19:29}(0T10)Nb1d2 / {15:18}(0T10)Qd8b6 
11.{15:31}(0T11)b2b4 / {13:40}(0T11)e7e6 
12.{15:13}(0T12)Qd3e3 / {7:27}(0T12)Bf8e7 
13.{12:22}(0T13)f2f3 / {5:34}(0T13)Qb6b5 
14.{10:31}(0T14)a2a4 / {4:39}(0T14)Qb5>>(0T11)e2 
15.{10:23}(-1T12)Qd3e2 / {4:39}(-1T12)Bh5e2 
16.{10:23}(-1T13)Ke1e2 / {3:52}(-1T13)Nf6e4 
17.{10:23}(0T15)Qe3>(-1T14)e4 / {3:52}(-1T14)d5e4 
18.{10:00}(-1T15)c3c4 / {3:37}(-1T15)Qb6d4 (0T15)Nf6e4 
19.{4:33}(0T16)Nd2e4 (-1T16)Bf4b8 / {3:11}(-1T16)Ra8b8 (0T16)d5e4 
20.{3:08}(-1T17)Bh3d7 (0T17)Bf4c7 / {3:11}(-1T17)Qd4d7 (0T17)Be7h4 
21.{2:00}(-1T18)Nd2>>(-1T17)d4 / {2:13}(1T17)Rb8d8 
22.{2:00}(1T18)Bh3>(0T18)h4 / {1:01}(-1T18)Qd7>(0T18)c7 (1T18)Rd8d4 
23.{1:29}(-1T19)Rh1d1 (0T19)Bh4g3 (1T19)Ra1d1 / {0:35}(1T19)Rd4d2 (0T19)Qc7>(-1T19)b6 
24.{1:15}(1T20)Rd1d2 (-1T20)Rd1d8 (0T20)Bg3c7 / {0:27}(-1T20)Rb8d8 (0T20)Ke8g8 (1T20)e7e6 
25.{0:47}(-1T21)Ra1d1 (0T21)Bh3g2 (1T21)Rh1d1 / {0:25}(-1T21)Rd8d1 (1T21)Bf8e7 (0T21)Rf8e8 
26.{0:52}(0T22)Bg2>>(0T17)g7 / {0:25}(2T17)Rh8g8 
27.{0:52}(2T18)Rh1g1 / {0:25}(2T18)Bh5g6 
28.{0:57}(2T19)Bg7e5 / {0:26}(0T22)Bh5>>(0T15)a5 
29.{0:48}(-2T16)Bf4e5 / {0:26}(-2T16)Nf6d7 
30.{0:48}(-2T17)Bh3e6 / {0:26}(-2T17)Nd7e5 
31.{0:48}(-2T18)Be6c8 / {0:26}(-2T18)Ra8c8 
32.{0:53}(-2T19)b4a5 / {0:31}(-2T19)Bh5>>(-2T16)e5 
33.{0:53}(-3T17)d4e5 / {0:31}(-3T17)Ba5b6 
34.{0:53}(-3T18)Nd2c4 / {0:31}(-3T18)d5c4 
35.{0:50}(-3T19)Bh3e6 
)", R"(
1.{20:05}(0T1)Ng1f3 / {20:05}(0T1)Ng8f6 
2.{20:05}(0T2)c2c3 / {20:05}(0T2)d7d5 
3.{20:05}(0T3)e2e4 / {20:05}(0T3)Nb8c6 
4.{19:17}(0T4)Nf3g5 / {20:00}(0T4)e7e5 
5.{17:29}(0T5)d2d4 / {19:56}(0T5)Qd8e7 
6.{15:01}(0T6)d4e5 / {17:50}(0T6)Bc8f5 
7.{14:11}(0T7)Bc1e3 / {17:06}(0T7)Nf6g4 
8.{11:46}(0T8)Qd1f3 / {17:06}(0T8)Ng4e3 
9.{11:04}(0T9)Qf3e3 / {17:02}(0T9)Qe7g5 
10.{11:04}(0T10)e4f5 / {16:28}(0T10)Qg5e3 
11.{10:40}(0T11)f2e3 / {16:15}(0T11)Bf8c5 
12.{10:21}(0T12)Bf1e2 / {16:08}(0T12)Ke8c8 
13.{6:46}(0T13)Nb1d2 / {16:08}(0T13)Bc5e3 
14.{6:13}(0T14)Ke1c1 / {16:06}(0T14)d5d4 
15.{5:50}(0T15)c3c4 / {15:42}(0T15)d4d3 
16.{1:32}(0T16)Be2d3 / {15:42}(0T16)Rd8d3 
17.{1:32}(0T17)Kc1c2 / {15:42}(0T17)Nc6b4 
18.{1:28}(0T18)Kc2c1 / {15:42}(0T18)Rh8d8 
19.{1:28}(0T19)Rh1e1 / {15:42}(0T19)Rd3d2 
20.{1:16}(0T20)Re1e3 / {15:47}(0T20)Rd2d1 
21.{1:17}(0T21)Kc1>>(0T20)b1 / {15:47}(1T20)Nb4d3 
22.{1:04}(1T21)Kc1>>(0T20)b1 / {15:43}(1T21)Nd3b2 (0T21)Rd1e1 
)"
    };
    std::vector<std::vector<full_move>> mvss;
    int count = 0;
    int n = 200;
    for(auto pgn : pgns)
    {
        auto mvs = pgn_to_moves_(pgn);
        mvss.push_back(mvs);
        count += mvs.size();
    }
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < n; i++)
    {
        for(auto mvs : mvss)
        {
            run_game(mvs);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "totally " << pgns.size()*n << " games ran, took " << duration << " ms\n";
    std::cout << "average time per game: " << duration/(pgns.size()*n) << " ms\n";
    std::cout << "average time per move: " << duration/(count*n) << " ms\n";
    
    //test1();
    std::cout << "---= run_games.cpp: all tests passed =---" <<std::endl;
    return 0;
}
