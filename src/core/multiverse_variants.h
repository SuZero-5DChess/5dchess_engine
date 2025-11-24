#ifndef MULTIVERSE_VARIANTS_H
#define MULTIVERSE_VARIANTS_H

#include "multiverse_base.h"

class multiverse_odd : public multiverse
{
    std::pair<int,int> calculate_active_range() const override;
public:
    using multiverse::multiverse;
    multiverse_odd(std::string input, int size_x = BOARD_LENGTH, int size_y = BOARD_LENGTH);
    multiverse_odd(std::vector<boards_info_t> boards, int size_x = BOARD_LENGTH, int size_y = BOARD_LENGTH);
    std::unique_ptr<multiverse> clone() const override
    {
        return std::make_unique<multiverse_odd>(*this);
    }
    std::string pretty_lt(vec4 p0) const override
    {
        return "(" + std::to_string(p0.l()) + "T" + std::to_string(p0.t()) + ")";
    }
};

class multiverse_even : public multiverse
{
    std::pair<int,int> calculate_active_range() const override;
public:
    using multiverse::multiverse;
    multiverse_even(std::string input, int size_x = BOARD_LENGTH, int size_y = BOARD_LENGTH);
    multiverse_even(std::vector<boards_info_t> boards, int size_x = BOARD_LENGTH, int size_y = BOARD_LENGTH);
    std::unique_ptr<multiverse> clone() const override
    {
        return std::make_unique<multiverse_even>(*this);
    }
    std::string pretty_lt(vec4 p0) const override
    {
        std::string sign = p0.l() >= 0 ? "+" : "-";
        int l_abs = p0.l() >= 0 ? p0.l() : ~p0.l();
        return "(" + sign + std::to_string(l_abs) + "T" + std::to_string(p0.t()) + ")";
    }
};

#endif /* MULTIVERSE_VARIANTS_H */
