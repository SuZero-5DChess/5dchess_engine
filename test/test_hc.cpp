#include <iostream>
#include <string>
#include <regex>

#include "hypercuboid.h"

std::string t0_fen = ""
"[Size 8x8]"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:0:b]\n"
"[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]\n";


int main()
{
    state s{multiverse(t0_fen)};
    std::cout << "starting_test:\n";
    auto w = HC_search(s);
    for(auto it = w.begin(); it != w.end(); ++it)
    {
        auto x = *it;
        if(x.size() == 0)
        {
            std::cout << "No avilable action in this turn";
        }
        else
        {
            std::cout << "Valid action with " << x.size() << " moves: ";
            for(full_move m : x)
            {
                std::cout << m << " ";
            }
        }
        std::cout << "\n";
        //break;
        std::cout << "\n----------------------------\n\n";
    }
    
    return 0;
}
