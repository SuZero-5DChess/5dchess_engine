// see: https://coliru.stacked-crooked.com/a/c81d5b84c757ce38
#include <vector>
#include <tuple>

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

void print_range(auto const rem, auto const& range)
{
    for (std::cout << rem; auto const& elem : range)
        std::cout << elem << ' ';
    std::cout << '\n';
}
