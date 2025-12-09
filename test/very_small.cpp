#include "game.h"
#include <tuple>
#include <ranges>

std::string very_small_open =
R"(
[Size "4x4"]
[Board "custom"]
[Mode "5D"]
[nbrk/3p*/P*3/KRBN:0:1:w]
)";
int main()
{
    game g = game::from_pgn(very_small_open);
    std::cout << g.get_current_state().to_string();
    ext_move m(vec4(0,1,1,0), vec4(0,2,1,0));
    g.apply_move(m);
    g.submit();
    g.suggest_action();
    g.suggest_action();
    bool flag = g.suggest_action();
    std::cout << flag << "\n";
    for(auto & [act, txt] : g.get_child_moves())
    {
        std::cout << txt << "\n";
        std::cout << g.get_current_state().pretty_action(act) << "\n";
    }
    std::cout << g.show_pgn() << "\n";
    return 0;
}
