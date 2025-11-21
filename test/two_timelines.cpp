#include "state.h"

std::string str = R"(
[Mode "5D"]
[Board "Custom - Even"]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:+0:0:b]
[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:-0:0:b]
(1.(L-0)Nf3 (L+0)e3 / (L+0)Nf6)
1.(L-0)e3 (L+0)N>>g2 {this is branching!}
)";

int main()
{
    pgnparser_ast::game g = *pgnparser(str).parse_game();
    std::cout << g << "\n\n" << std::endl;
    state s(g);
    std::cout << s.to_string();
    return 0;
}
