#ifndef MULTIVERSE_H
#define MULTIVERSE_H

#include "multiverse_base.h"
#include "multiverse_variants.h"

inline std::pair<int, int> next_tc(int t, int c)
{
    int v = (t << 1 | c) + 1;
    return std::make_pair(v >> 1, v & 1);
}

#endif /* MULTIVERSE_H */
