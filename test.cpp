#include <iostream>
#include "bitboard.h"

 int main()
 {
     bitboard_t bb = 0b1011;
     std::cout << bb_get_pos(bb) << std::endl;
     print_range("bits: ", marked_pos(bb));
     std::cout << "shutting down" << std::endl;
     return 0;
 }
