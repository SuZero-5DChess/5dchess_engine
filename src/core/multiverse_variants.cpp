#include "multiverse_variants.h"

multiverse_odd::multiverse_odd(std::string input, int size_x, int size_y)
    : multiverse(std::move(input), size_x, size_y)
{
    update_active_range();
}

multiverse_odd::multiverse_odd(std::vector<boards_info_t> boards, int size_x, int size_y)
    : multiverse(boards, size_x, size_y)
{
    update_active_range();
}

std::pair<int, int> multiverse_odd::calculate_active_range() const
{
    auto [l_min, l_max] = multiverse::get_lines_range();
    int tmp = std::min(-l_min, l_max);
    int l = tmp + ((tmp < std::max(-l_min, l_max)) ? 1 : 0);
    int active_min = std::max(l_min, -l);
    int active_max = std::min(l_max, l);
    return std::make_pair(active_min, active_max);
}



multiverse_even::multiverse_even(std::string input, int size_x, int size_y)
    : multiverse(std::move(input), size_x, size_y)
{
    update_active_range();
}

multiverse_even::multiverse_even(std::vector<boards_info_t> boards, int size_x, int size_y)
    : multiverse(boards, size_x, size_y)
{
    update_active_range();
}

std::pair<int, int> multiverse_even::calculate_active_range() const
{
    auto [l_min0, l_max] = multiverse::get_lines_range();
    int l_min = l_min0 + 1; // the only difference
    int tmp = std::min(-l_min, l_max);
    int l = tmp + ((tmp < std::max(-l_min, l_max)) ? 1 : 0);
    int active_min = std::max(l_min, -l);
    int active_max = std::min(l_max, l);
    return std::make_pair(active_min, active_max);
}
