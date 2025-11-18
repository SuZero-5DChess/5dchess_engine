#ifndef DEBUG_H
#define DEBUG_H

/*
A simple debug printing utility.
To enable debug printing, define DEBUGMSG in the .cpp file before including this header.
Example:
    #define DEBUGMSG
    #include "debug.h"
*/
namespace dprint_detail {
template <typename T>
void debug_print_impl(T t)
{
    std::cerr << t << "\n";
}

template<typename T, typename ...Args>
void debug_print_impl(T t, Args... args)
{
    std::cerr << t << " ";
    debug_print_impl(args...);
}

template<typename ...Args>
void debug_print(Args... args)
{
    std::cerr << "[DEBUG] ";
    debug_print_impl(args...);
}
} // namespace


#ifdef DEBUGMSG
#define dprint(...) (dprint_detail::debug_print(__VA_ARGS__))
#else
#define dprint(...)
#endif

#else
#error "debug.h included multiple times"
/*
IMPORTANT: This file is supposed to be included in ONLY ONE translation unit (i.e. .cpp file).
If you need to use debug printing in multiple .cpp files, define DEBUGMSG in each of them.

You should NOT include this file in a header file.
*/
#endif /* DEBUG_H */
