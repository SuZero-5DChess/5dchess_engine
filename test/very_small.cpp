#include "game.h"
#include <tuple>

std::string very_small_open = R"(
[Size "4x4"]
[Board "custom"]
[Mode "5D"]
[nbrk/3p*/P*3/KRBN:0:1:w]

1. (0T1)Bc1b2 /
)";

int main()
{
    game g(very_small_open);
    std::cout << g.get_current_state().m.to_string();
    std::cout << std::get<3>(g.get_current_state().m.get_boards()[0]) << "\n";
    
    vec4 p(3,2,1,0);
    std::cout << g.get_current_state().m.get_piece(p, 1) << "\n";
    for(auto mv : g.get_current_state().m.gen_piece_move(p, 1))
    {
        std::cout << mv << "\n";
    }
    return 0;
}
