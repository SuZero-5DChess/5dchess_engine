#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <tuple>
#include <iostream>

/*
The append/concatenate functions.
See: https://coliru.stacked-crooked.com/a/c81d5b84c757ce38
*/
template<typename T, typename ...Args>
void append_vectors(std::vector<T>& res, const Args&... args)
{
    using std::begin;
    using std::end;
    
    (res.insert(res.end(), begin(args), end(args)), ...);
}

template<typename ...Args>
auto concat_vectors(const Args&... args)
{
    using T = typename std::tuple_element<0, std::tuple<Args...>>::type::value_type;
    std::vector<T> res;
    res.reserve((... + args.size()));
    append_vectors(res, args...);
    return res;
}

/*
The tuple printer function. (For debugging.)
See: https://geo-ant.github.io/blog/2020/stream-insertion-for-tuples/
*/
namespace detail
{
    template<typename ...Ts, size_t ...Is>
    std::ostream & println_tuple_impl(std::ostream& os, std::tuple<Ts...> tuple, std::index_sequence<Is...>)
    {
        static_assert(sizeof...(Is)==sizeof...(Ts),"Indices must have same number of elements as tuple types!");
        static_assert(sizeof...(Ts)>0, "Cannot insert empty tuple into stream.");
        auto last = sizeof...(Ts)-1; // assuming index sequence 0,...,N-1
        
        return ((os << (Is == 0 ? "(" : "") << std::get<Is>(tuple) << (Is != last ? "," : ")")),...);
    }
}

template<typename ...Ts>
std::ostream & operator<<(std::ostream& os, const std::tuple<Ts...> & tuple)
{
    return detail::println_tuple_impl(os, tuple, std::index_sequence_for<Ts...>{});
}

template <typename A, typename B>
std::ostream& operator<<(std::ostream& os, const std::pair<A,B> x)
{
    return os << "(" << x.first << "," << x.second << ")";
}

/*
The list printer function. (For debugging.)
*/
void print_range(auto const rem, auto const& range)
{
    std::cout << rem;
    for(auto const& elem : range)
        std::cout << elem << ' ';
    std::cout << '\n';
}

#define SHOW(s) std::cout << #s << ": " << (s) << std::endl;

/*
helper type for the visitor
See: https://en.cppreference.com/w/cpp/utility/variant/visit
*/

template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };
 
/*
 static array generator
 usage:
 constexpr std::array<%your_type%,%length%> =
     generate_array(std::make_index_sequence<%length%>{},fuction size_t -> %your_type%)
 */
template <std::size_t... N, typename F>
constexpr auto generate_array(std::index_sequence<N...>, F f)
-> std::array<decltype(f(std::declval<size_t>())), sizeof...(N)>
{
    return {f(N)...};
}

#endif // UTILS_H
