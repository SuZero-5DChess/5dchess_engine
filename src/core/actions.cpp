#include "actions.h"
#include <regex>
#include <sstream>
#include "utils.h"


full_move::full_move(std::string str): from{0,0,0,0}, to{0,0,0,0}
{
    std::regex pattern1(R"(\((-?\d+)T(-?\d+)\)[A-Z]?([a-h])([1-8])([a-h])([1-8]))");
    std::regex pattern2(R"(\((-?\d+)T(-?\d+)\)[A-Z]?([a-h])([1-8])>>?\(T(-?\d+)\)([a-h])([1-8]))");
    std::regex pattern3(R"(\((-?\d+)T(-?\d+)\)[A-Z]?([a-h])([1-8])>>?\((-?\d+)T(-?\d+)\)([a-h])([1-8]))");
    std::smatch match;
    int l1, t1, x1, y1;
    int l2, t2, x2, y2;
    
    if(std::regex_match(str, match, pattern1))
    {
        l1 = l2 = std::stoi(match[1]);
        t1 = t2 = std::stoi(match[2]);
        x1 = static_cast<int>(match[3].str()[0] - 'a');
        y1 = static_cast<int>(match[4].str()[0] - '1');
        x2 = static_cast<int>(match[5].str()[0] - 'a');
        y2 = static_cast<int>(match[6].str()[0] - '1');
    }
    else if(std::regex_match(str, match, pattern2))
    {
        l1 = l2 = std::stoi(match[1]);
        t1 = std::stoi(match[2]);
        x1 = static_cast<int>(match[3].str()[0] - 'a');
        y1 = static_cast<int>(match[4].str()[0] - '1');
        t2 = std::stoi(match[5]);
        x2 = static_cast<int>(match[6].str()[0] - 'a');
        y2 = static_cast<int>(match[7].str()[0] - '1');
    }
    else if(std::regex_match(str, match, pattern3))
    {
        l1 = std::stoi(match[1]);
        t1 = std::stoi(match[2]);
        x1 = static_cast<int>(match[3].str()[0] - 'a');
        y1 = static_cast<int>(match[4].str()[0] - '1');
        l2 = std::stoi(match[5]);
        t2 = std::stoi(match[6]);
        x2 = static_cast<int>(match[7].str()[0] - 'a');
        y2 = static_cast<int>(match[8].str()[0] - '1');
    }
    else
    {
        throw std::runtime_error("Cannot match this move in any known pattern: " + str);
    }
    from = vec4(x1, y1, t1, l1);
    to = vec4(x2, y2, t2, l2);
}

std::string full_move::to_string() const
{
    std::ostringstream os;
    vec4 p = from, q = to;
    vec4 d = q - p;
    if(d.l() == 0)
    {
        if(d.t() == 0)
        {
            os << '(' << p.l() << 'T' << p.t() << ')' << (char)(p.x()+'a') << (char)(p.y()+'1') << (char)(q.x()+'a') << (char)(q.y()+'1');
        }
        else
        {
            os << '(' << p.l() << 'T' << p.t() << ')' << (char)(p.x()+'a') << (char)(p.y()+'1') << ">(T" << q.t() << ')' << (char)(q.x()+'a') << (char)(q.y()+'1');
        }
    }
    else
    {
        os << '(' << p.l() << 'T' << p.t() << ')' << (char)(p.x()+'a') << (char)(p.y()+'1') << ">(" << q.l() << 'T' << q.t() << ')' << (char)(q.x()+'a') << (char)(q.y()+'1');
    }
    return os.str();
}

bool full_move::operator<(const full_move &other) const
{
    return std::tie(from, to) < std::tie(other.from, other.to); 
}

bool full_move::operator==(const full_move &other) const
{
    return std::tie(from, to) == std::tie(other.from, other.to);
}

/*********************************/

move5d::move5d() : data(std::monostate()) {}

move5d::move5d(const vec4& p, const vec4& q) : data(full_move(p,q)) {}

move5d::move5d(std::string str)
{
    data = full_move(str);
}

move5d move5d::submit() { return move5d(); }

move5d move5d::move(const vec4& p, const vec4& d) { return move5d(p, d); }

bool move5d::nonempty()
{
    return data.index();
}

bool move5d::operator<(const move5d& other) const 
{
    return data < other.data;
}

bool move5d::operator==(const move5d& other) const 
{
    return data == other.data;
}

std::string move5d::to_string() const
{
    switch (data.index())
    {
        case 0:
            return "Submit";
        case 1:
            return std::get<full_move>(data).to_string();
        default:
            throw std::runtime_error("Unknown move5d variant");
    }
}

std::ostream &operator<<(std::ostream &os, const full_move &fm)
{
    os << fm.to_string();
    return os;
}

std::ostream &operator<<(std::ostream &os, const move5d &m)
{
    os << m.to_string();
    return os;
}

actions::actions(move5d m) : value(m) {}

actions::actions(move5d m, std::set<actions> s) : value(std::make_tuple(m, std::move(s))) {}

actions actions::leaf(move5d m) { return actions(m); }

actions actions::branch(move5d m, std::set<actions> s) { return actions(m, std::move(s)); }

bool actions::operator<(const actions& other) const { return value < other.value; }

bool actions::operator==(const actions& other) const { return value == other.value; }

std::ostream& operator<<(std::ostream& os, const actions& a)
{
    return a.pretty_print(os, "", 0);
}

/*********************************/

const static std::string shapes[] = { "", "", "├── ", "│   ", "└── " , "    " };

std::ostream& actions::pretty_print(std::ostream& os, const std::string& prefix, const int& now) const
{
    if (std::holds_alternative<move5d>(value))
    {
        os << prefix << shapes[now] << std::get<move5d>(value) << "\n";
    }
    else
    {
        const auto& [m, set] = std::get<std::tuple<move5d, std::set<actions>>>(value);
        os << prefix << shapes[now] << m << "\n";
        auto it = set.begin();
        while (it != set.end())
        {
            bool is_last = (std::next(it) == set.end());
            std::string new_prefix = prefix + shapes[now | 0x1];
            it->pretty_print(os, new_prefix, (is_last ? 4 : 2));
            ++it;
        }
    }
    return os;
}
