#include "bitboard.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "utils.h"
#include "magic.h"

std::string bb_to_string(bitboard_t bb)
{
    std::ostringstream ss;
    ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << bb << std::endl;
    for (int y = BOARD_LENGTH - 1; y >= 0; y--)
    {
        for (int x = 0; x < BOARD_LENGTH; x++)
        {
            ss << ((bb & (static_cast<bitboard_t>(1)<<ppos(x, y))) ? "1 " : ". ");
        }
        ss << "\n";
    }
    return ss.str();
}


std::vector<int> marked_pos(bitboard_t b)
{
    std::vector<int> result;
    while(b)
    {
        int n = bb_get_pos(b);
        result.push_back(n);
        b &= ~(bitboard_t(1) << n);
    }
    return result;
}

constexpr std::array<bitboard_t, BOARD_SIZE> knight_attack_data = generate_array(std::make_index_sequence<BOARD_SIZE>{}, [](size_t pos) -> bitboard_t
{
    bitboard_t z = bitboard_t(1) << pos, bb = 0;
    bb |= shift_north(shift_northwest(z));
    bb |= shift_west(shift_northwest(z));
    bb |= shift_north(shift_northeast(z));
    bb |= shift_east(shift_northeast(z));
    bb |= shift_south(shift_southwest(z));
    bb |= shift_west(shift_southwest(z));
    bb |= shift_south(shift_southeast(z));
    bb |= shift_east(shift_southeast(z));
    return bb;
});

constexpr std::array<bitboard_t, BOARD_SIZE> king_attack_data = generate_array(std::make_index_sequence<BOARD_SIZE>{}, [](size_t pos) -> bitboard_t
{
    bitboard_t z = bitboard_t(1) << pos, bb = 0;
    bb |= shift_north(z);
    bb |= shift_south(z);
    bb |= shift_west(z);
    bb |= shift_east(z);
    bb |= shift_northwest(z);
    bb |= shift_northeast(z);
    bb |= shift_southwest(z);
    bb |= shift_southeast(z);
    return bb;
});
                                                                        
bitboard_t knight_attack(int pos)
{
    return knight_attack_data[pos];
}

bitboard_t king_attack(int pos)
{
    return king_attack_data[pos];
}
