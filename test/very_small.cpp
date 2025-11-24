#include "game.h"
#include <tuple>
#include <ranges>

std::string very_small_open = R"(
[Size "4x4"]
[Board "custom"]
[Mode "5D"]
[nbrk/3p*/P*3/KRBN:0:1:w]

1. (0T1)d1c3 /

)";

int main()
{
    game g(very_small_open);
    g.apply_move(move5d("(0T1)d3d1"));
    std::cout << g.get_current_state().to_string();
//    print_range("Checking moves: ", g.get_current_state().find_checks());
//    std::cout << "Checking moves: ";
//    auto [t, c] = g.get_current_present();
//    for(full_move fm : g.get_current_state().find_checks())
//    {
//        std::cout << g.get_current_state().pretty_move(fm, 1-c) << " ";
//        std::cout << static_cast<int>(g.get_current_state().get_piece(fm.to, 1-c));
//    }
//    std::cout << std::endl;
//    g.apply_move(move5d::submit());
//    auto mv = g.get_current_state().parse_move("a3");
//    std::cout << mv << std::endl;
    return 0;
}
