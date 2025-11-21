#include "game.h"
#include <tuple>
#include <ranges>

std::string str = R"(
[Board "custom"]
[Mode "5D"]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]

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
)";

int main()
{
    //game g(str);
    //std::cout << g.get_current_state().to_string();
    std::map<std::string,std::string> headers;
    headers.insert({"Mode", "5D"});
    auto [_, success] = headers.insert({"Mode", "6D"});
    std::cout << success << "\n";
    
    std::cout << sizeof(multiverse) << "\n";
    std::cout << sizeof(std::unique_ptr<multiverse>) << "\n";
    
    std::cout << sizeof(state) << "\n";
    return 0;
}
