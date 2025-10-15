#include "geometry.h"
#include <cassert>
#include <sstream>
#include "utils.h"

const std::set<int> &HC::operator[](size_t i) const
{
    return axes[i];
}

bool HC::contains(point loc) const
{
    assert(loc.size() == axes.size());
    for(int i = 0; i < axes.size(); i++)
    {
        if(!axes[i].contains(loc[i]))
            return false;
    }
    return true;
}

bool search_space::contains(point loc) const
{
    for(const auto& hc : hcs)
    {
        if(hc.contains(loc))
            return true;
    }
    return false;
}

void search_space::concat(search_space &&other)
{
    hcs.splice(hcs.end(), other.hcs);
}

search_space HC::remove_slice(const slice &s) const
{
    search_space result;
    for(const auto& [i, fixed_coords] : s.fixed_axes)
    {
        HC x = *this;
        x.axes[i] = set_minus(x.axes[i], fixed_coords);
        result.hcs.push_back(std::move(x));
    }
    return result;
}

search_space HC::remove_point(const point &p) const
{
    search_space result;
    for(size_t i = 0; i < p.size(); i++)
    {
        HC x = *this;
        x.axes[i].erase(p[i]);
        result.hcs.push_back(std::move(x));
    }
    return result;
}

std::string HC::to_string(bool verbose) const
{
    std::ostringstream oss;
    if(verbose)
    {
        oss << "Hypercuboid with " << axes.size() << " axes:\n";
    }
    for(size_t i = 0; i < axes.size(); i++)
    {
        oss << " Axis " << i << ": {";
        for(int v : axes[i])
        {
            oss << v << ", ";
        }
        oss.seekp(-2, oss.cur);
        oss << "}\n";
    }
    return oss.str();
}

std::string slice::to_string() const
{
    std::ostringstream oss;
    oss << "Slice with " << fixed_axes.size() << " axes fixed: \n";
    for(const auto& [k, v] : fixed_axes)
    {
        oss << "On axis " << k << ", fixing {";
        for(int i : v)
        {
            oss << i << ", ";
        }
        oss.seekp(-2, oss.cur);
        oss << "}\n";
    }
    return oss.str();
}

std::string search_space::to_string() const
{
    std::string result = "Search space: total " + std::to_string(hcs.size()) + " hypercuboids  \n";
    for(const auto& hc : hcs)
    {   
        result += hc.to_string(false);
        result += "&\n";
    }
    result.pop_back();
    result.pop_back();
    return result;
}
