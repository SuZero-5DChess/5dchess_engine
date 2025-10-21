#include "game.h"
#include <tuple>

std::string very_small_open = R"(
[Size "4x4"]
[Board "custom"]
[Mode "5D"]
[nbrk/3p*/P*3/KRBN:0:1:w]

1. (0T1)c1b2 / (0T1)d3d2

)";

int main()
{
    game g(very_small_open);
    std::cout << g.get_current_state().to_string();
    print_range("Checking moves: ", g.get_current_state().find_checks());
//    std::vector<int> v1{1}, v2{4,5,6,7};
//    append_vectors(v1, v2);
//    print_range("", v1);
//    std::cout << std::get<3>(g.get_current_state().get_boards()[0]) << "\n";
//    
//    vec4 p(3,2,1,0);
//    std::cout << g.get_current_state().get_piece(p, 1) << "\n";
//    for(auto mv : g.get_current_state().gen_piece_move(p))
//    {
//        std::cout << mv << "\n";
//    }
    return 0;
}
