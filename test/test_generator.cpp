#include "generator.h"
#include <iostream>

template<std::integral T>
generator<T> range(T first, const T last)
{
    while (first < last)
    {
        std::cerr << "generating " << first << " ";
        if(first == 80)
            co_return;
        co_yield first++;
    }
}

void test_range()
{
    auto g = range(65,99);
    std::cerr << "phase 1: ";
    if(auto k = g.first())
    {
        std::cerr << "got " << k.value() << "\n";
    }
    else
    {
        std::cerr << "got nothing\n";
    }
    std::cerr << "phase 2: ";
    if(auto k = g.first())
    {
        std::cerr << "got " << k.value() << "\n";
    }
    else
    {
        std::cerr << "got nothing\n";
    }
    std::cerr << "phase 3: ";
    for(auto l : g)
    {
        std::cerr << "then got " << l << "\n";
    }
    std::cerr << "test_range finished\n";
}

int main()
{
    test_range();
    std::cerr << "---= test_generator.cpp: all passed =---" << std::endl;
    return 0;
}