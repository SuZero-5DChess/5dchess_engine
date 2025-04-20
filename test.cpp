#include <iostream>
#include <string>
#include "board.h"
#include <regex>

int main()
{
    std::regex size_regex(R"(^\s*(\d+)\s*x\s*(\d+)\s*$)");
    std::string size = "  8 x 9";
    std::smatch size_sm;
    if(std::regex_search(size, size_sm, size_regex))
    {
        int size_x = std::stoi(size_sm[1]);
        int size_y = std::stoi(size_sm[2]);
        std::cout << size_x << ", " << size_y << std::endl;
    }
    else
    {
        std::cout << "incorrect" <<std::endl;
    }
    return 0;
}
