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


std::vector<move5d> pgn_to_moves_(const std::string& input)
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
    std::vector<move5d> result;
    std::istringstream result_stream(output);
    std::string word;
    //remove the first "submit"
    result_stream >> word;
    
    multiverse_odd m0(t0_fen);
    state s(m0);
    try {
        while (result_stream >> word)
        {
            if(word.compare("submit") != 0)
            {
                auto [m, pt, candidates] = s.parse_pgn(word);
                if(!m.has_value())
                {
                    std::cout << "pgn: " << word << " is ";
                    if(candidates.size() > 0)
                    {
                        std::cout << "ambiguious, possible candidates are:\n";
                        for(full_move fm : candidates)
                        {
                            std::cout << "  " << s.pretty_move(fm) << "\n";
                        }
                    }
                    else
                    {
                        std::cout << "invalid, no matches found.\n";
                    }
                    throw 0;
                }
                result.push_back(move5d(*m));
                bool flag = s.apply_move(*m);
                if(!flag)
                {
                    throw 1;
                }
            }
            else
            {
                result.push_back(move5d::submit());
                bool flag = s.submit();
                if(!flag)
                {
                    throw 2;
                }
            }
        }
    }
    catch(int err)
    {
        std::cout << "pgn_to_moves_:Error " << err << " from\n";
        std::cout << input;
        std::cout << "Cannot parse w.r.t. the current state:\n";
        std::cout << s.to_string() << std::endl;
        exit(1);
    }
    return result;
}

void test1()
{
    game g(t0_fen);
//     move5d mv("(0T1)Ng1f3");
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
    std::vector<move5d> mvs = {
        move5d("(0T1)e2e3"),
        move5d::submit(),
        move5d("(0T1)g8f6"),
        move5d::submit(),
        move5d("(0T2)f1c4"),
        move5d::submit(),
        move5d("(0T2)g7g5"),
        move5d::submit(),
        move5d("(0T3)g1h3"),
        move5d::submit(),
        move5d("(0T3)g5g4"),
        move5d::submit(),
        move5d("(0T4)e1g1"),
        move5d::submit(),
    };
    
    for(move5d mv : mvs)
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
//     std::cout << g.apply_move(move5d("(0T2)Rh8g8")) << endl;
    std::cout << g.get_current_state().to_string();
    std::cout << "test1 finished" << std::endl;
}


multiverse_odd m0(t0_fen);

template<bool DETECT_CHECKS>
void run_game(std::vector<move5d> mvs)
{
    state s(m0);
    
//    std::cerr << "Running new game ..." << std::endl;
    
    for(move5d mv : mvs)
    {
        bool flag;
//        std::cerr << "apply " << mv << std::endl;
        if(std::holds_alternative<std::monostate>(mv.data))
        {
            flag = s.submit();
        }
        else if(std::holds_alternative<full_move>(mv.data))
        {
            auto fm = std::get<full_move>(mv.data);
            flag = s.apply_move(fm);
        }
        else
        {
            throw std::runtime_error("Unknown move5d variant.\n");
        }
        if(!flag)
        {
            std::cerr << "In run_game:\n";
            std::cerr << "current boards:\n";
            std::cerr << s.to_string() << std::endl;
            std::cerr << "failed to apply: " << mv << "\n";
            std::visit(overloads {
                [&](std::monostate){},
                [&](full_move data)
                {
                    vec4 p = data.from;
                    std::cerr << "The allowed moves are: ";
                    for(vec4 d : s.gen_piece_move(p))
                    {
                        std::cerr << move5d::move(p, d) << " ";
                    }
                    std::cerr << std::endl;
                }
            }, mv.data);
            exit(-1);
        }
        if constexpr(DETECT_CHECKS)
        {
            if(s.find_checks().first())
            {
                std::cerr << "In run_game:\n";
                std::cerr << "current boards:\n";
                std::cerr << s.to_string() << std::endl;
                std::cerr << "This move" << mv << "is illeagal because hostile is checking.\n";
            }
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
)", R"(
1.e3/Nf6
2.b4/e6
3.Qf3/Bxb4
4.Qg3/Bf8
5.Nf3/d5
6.Nc3/c6
7.a4/Na6
8.Bxa6/bxa6
9.Ba3/g6
10.Bxf8/Rxf8
11.Qg5/Qe7
12.Nd1/a5
13.Nd4/Ba6
14.Qe5/Rc8
15.Rb1/Nd7
16.Qg3/Qc5
17.c3/Nf6
18.Qf3/Ne4
19.Qg4/Qd6
20.Rb2/Nc5
21.Rb6/Nd3
22.Ke2/Ne5
23.Rxa6/Nxg4
24.Nxe6/Qxe6
25.Rxc6/Rxc6
26.Nb2/(0T26)Ng4>>(0T25)e4
27.(0T27)Nb2>>(0T25)c2/(1T25)Qxe3
)", R"(
1.Nf3/Nf6
2.d4/d5
3.c3/c6
4.Bf4/Bf5
5.e3/e6
6.Bd3/Bxd3
7.Qxd3/Bd6
8.Bxd6/Qxd6
9.Nbd2/Nbd7
10.a4/a6
11.Ra3/c5
12.dxc5/Nxc5
13.Qd4/Rd8
14.a5/Rd7
15.Nb3/Nfe4
16.Nxc5/Nxc5
17.b4/Ne4
18.Qa7/Rd8
19.Qxb7/Kg8
20.Ne5/Qxe5
21.(0T21)Qb7>>(0T18)e7/(1T18)Kxe7
22.(1T19)Qxg7/(0T21)Qe5>>x(0T18)h2
23.(-1T19)Qb8/(-1T19)Ke8(0T18)f8
24.(-2T19)Qb8
)", R"(
1.Nf3/Nf6
2.b3/d5
3.Bb2/Bg4
4.e3/Nbd7
5.Nc3/e6
6.Bd3/Nc5
7.Qe2/c6
8.Qf1/Nce4
9.h3/Bh5
10.Bxe4/Nxe4
11.Nxe4/dxe4
12.Nd4/Qa5
13.Bc3/Qc7
14.Qc4/Qd7
15.g4/Bg6
16.Nf5/b5
17.Qxe4/Qd5
18.Qxd5/cxd5
19.Rg1/Rc8
20.Nxg7/Bxg7
21.(0T21)Bc3>>x(0T18)c6/(1T18)Kd8
22.(1T19)Ba5/(1T19)Kd8>>(0T19)d8
23.(-1T20)Ba5 (1T20)Qxd5
)",R"(
1.e3/Nf6
2.Nf3/d5
3.c4/c6
4.cxd5/Qxd5
5.Nc3/Qh5
6.Qb3/e6
7.Nb5/Na6
8.Nbd4/Nc5
9.Qc4/Bd7
10.Bd3/Nxd3
11.Qxd3/Be7
12.b3/a6
13.Bb2/c5
14.Ne2/Bb5
15.Qc2/Qg6
16.Qxg6/hxg6
17.Ng3/Rd8
18.a4/Bc6
19.Rd1/Bd5
20.Kg1/Bxb3
21.Rde1/Bxa4
22.e4/Bc6
23.Be5/Bd6
24.d4/Ng4
25.Bxg7/Rg8
26.e5/Rxg7
27.exd6/Rxd6
28.dxc5/Rd3
29.h3/Bxf3
30.gxf3/Nf6
31.Re3/Rxe3
32.fxe3/Nd5
33.e4/Ne3
34.Rc1/N>>xg3
35.(0T35)Rc1>>(0T26)c1/(1T26)Rxg7
36.(1T27)dxc5/(1T27)Bc7
37.(1T28)h3/(1T28)Nf6
38.(1T29)Red1/(1T29)Nxe4
39.(1T30)Nxe4/(1T30)Bxe4
40.(1T31)Rxd8/(1T31)Bxd8
41.(1T32)Rce1
)",R"(
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
23. (0T23)Kg1>>(0T22)g1 / (0T23)Ke6 (1T22)Rxh4
24. (1T23)Khg2 / (1T23)Rh3
25. (0T24)Rg5 (1T24)Rg5 / (0T24)Kf6 (1T24)Rh3>>(0T24)h3
26. (0T25)Rgg1 (1T25)Rg3 (-1T25)Rgg1 / (-1T25)Kd5 (0T25)Rh5 (1T25)Ke5
27. (-1T26)Rg1>>(-1T25)g1 / (2T25)Kf6
28. (0T26)Rge1 (2T26)R5g2 (1T26)Rga3 / (-1T26)Rh5 (2T26)Rhh8 (1T26)Kf6 (0T26)Kg7
29. (2T27)Rga2 (1T27)K1h1 (-1T27)Ra2 (0T27)Reb1 / (-1T27)Rh5>>(-1T25)h5
30. (-1T28)Ra1 (-2T26)Ra2 / (-1T28)Rc3 (-2T26)Kd5
31. (-1T29)Rh1 (-2T27)Rag2 / (-1T29)Rc3>>(-1T25)c3 (0T27)Rhh8
32. (-3T26)Raf1 / (-3T26)Kd5 (2T27)Ke5 (1T27)Kg7
33. (-3T27)Rf5 / (-3T27)Kc4 (-2T27)Kc4
34. (2T28)Rg1>(0T28)g1 (1T28)Rc3 (-2T28)Rg2>>(0T28)g2 (-3T28)Rgg5 / (0T28)Kh7 (3T28)Kh7 (2T28)R3h7 (1T28)Kh8 (-2T28)Rha5 (-3T28)Kb3
35. (1T29)Kh1>(2T29)g1 (-3T29)Rg3 (3T29)Rf2 (0T29)Rb2 (-2T29)Rg2 / (3T29)Kg7 (-2T29)Re4 (-3T29)Ka3 (1T29)Rb8 (0T29)Rhb8 (2T29)Rhb8
36. (-3T30)Rf5>(-1T30)f5 (-2T30)Rg1 (3T30)Ra5 (1T30)Rc3g3 (2T30)Ra2c2 (0T30)Rf2 /   (3T30)Rh8b8 (2T30)Ke5e6 (1T30)Rb8a8 (0T30)Rb8c8 (-1T30)Kd5e6 (-2T30)Re4e5 (-3T30)Ka3b2
37. (3T31)Ra5e5 (2T31)Rc2c3 (1T31)Rg3f3 (0T31)Rg1f1 (-1T31)Rf5a5 (-2T31)Rg1g2 (-3T31)Rg3g4 / (3T31)Ra8a1 (2T31)Rh7a7 (1T31)Ra8b8 (0T31)Kh7g8 (-1T31)Ke6f7 (-2T31)Ra5a1 (-3T31)Rh4h8
38. (3T32)Re5e3 (2T32)Ra1a2 (1T32)Kg2f2 (0T32)Ra1e1 (-1T32)Rh1f1 (-2T32)Rg2g1 (-3T32)Rg4g1 / (3T32)Ra1>>(3T30)a1 (2T32)Rb8c8 (1T32)Rb8a8 (0T32)Rc8c3 (-1T32)Kf7g8 (-2T32)Rh3h8 (-3T32)Rh3h7
39. (3T33)Rb1g1 (2T33)Ra2a6 (1T33)Rf3h3 (0T33)Rf2g2 (-1T33)Ra5g5 (-2T33)Rg1g4 (-3T33)Rg1g2 (-4T31)Ra5e5 / (2T33)Ke5 (1T33)Kg8  (-2T33)Kc3 (-3T33)Ka1 (-4T31)Rh8b8
40.  (2T34)Ra6h6 (1T34)Rh3h1   (-2T34)Rg4h4 (-3T34)Rg2h2 (-4T32)Rb1b3 / (2T34)Ra8b8 (1T34)Ra8b8 (-2T34)Ra8b8 (-3T34)Ra8b8 (-4T32)Kg7g8
41. (-4T33)Rb3g3 / (3T33)Kh8 (0T33)Kh8 (-1T33)Kh8 (-4T33)Kh8
42. (3T34)Rf2h2 (0T34)Rf1h1 (-1T34)Rf1h1 (-4T34)Rf2h2
)"
    };
    std::vector<std::vector<move5d>> mvss;
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
            run_game<false>(mvs);
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
