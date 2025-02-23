#include "actions.h"
#include <regex>

temp_move::temp_move() : data(std::monostate()) {}

temp_move::temp_move(const vec4& p, const vec4& d) : data(std::make_tuple(p, d)) {}

temp_move::temp_move(std::string str)
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
    vec4 p = vec4(x1, y1, t1, l1);
    data = std::make_tuple(p, vec4(x2, y2, t2, l2) - p);
}

temp_move temp_move::submit() { return temp_move(); }

temp_move temp_move::move(const vec4& p, const vec4& d) { return temp_move(p, d); }

bool temp_move::operator<(const temp_move& other) const 
{
    return data < other.data;
}

bool temp_move::operator==(const temp_move& other) const 
{
    return data == other.data;
}

std::ostream& operator<<(std::ostream& os, const temp_move& m)
{
    switch (m.data.index()) 
    {
        case 0:
            os << "Submit";
            break;
        case 1:
            auto [p, d] = std::get<std::tuple<vec4, vec4>>(m.data);
            vec4 q = p+d;
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
            break;
    }
    return os;
}

actions::actions(temp_move m) : value(m) {}

actions::actions(temp_move m, std::set<actions> s) : value(std::make_tuple(m, std::move(s))) {}

actions actions::leaf(temp_move m) { return actions(m); }

actions actions::branch(temp_move m, std::set<actions> s) { return actions(m, std::move(s)); }

bool actions::operator<(const actions& other) const { return value < other.value; }

bool actions::operator==(const actions& other) const { return value == other.value; }

std::ostream& operator<<(std::ostream& os, const actions& a)
{
    return a.pretty_print(os, "", 0);
}

const static std::string shapes[] = { "", "", "├── ", "│   ", "└── " , "    " };

std::ostream& actions::pretty_print(std::ostream& os, const std::string& prefix, const int& now) const
{
    if (std::holds_alternative<temp_move>(value))
    {
        os << prefix << shapes[now] << std::get<temp_move>(value) << "\n";
    }
    else
    {
        const auto& [m, set] = std::get<std::tuple<temp_move, std::set<actions>>>(value);
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
