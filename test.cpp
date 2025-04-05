#include <iostream>
#include "bitboard.h"

 int main()
 {
     int pos = 8*2+4, n = 3;
     std::cout << bb_to_string(queen_copy_mask(pos,n));
     std::cout << bb_to_string(bishop_copy_mask_data[(n<<6)|pos]);
    
     bitboard_t a, b, c, d, bb = 0;
     a = b = c = d = pmask(static_cast<int>(pos));
     for(int i = 0; i < n; i++)
     {
         a = shift_northwest(a);
         b = shift_northeast(b);
         c = shift_southwest(c);
         d = shift_southeast(d);
     }
     bb = a | b | c | d;
     std::cout << "bb: " << bb_to_string(bb);
//     std::cout << shift_northeast(a);
//     std::cout << "shutting down" << std::endl;
     return 0;
 }
