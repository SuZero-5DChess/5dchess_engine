//
//  vec4.h
//  engine
//
//  Created by ftxi on 2024/12/13.
//

#ifndef vec4_h
#define vec4_h

#include <cstdint>
#include <tuple>
#include <iostream>
#include <sstream>
#include <iomanip>

typedef std::uint32_t vec4_t;
const vec4_t L_BITS = 8, T_BITS = 8, Y_BITS = 8, X_BITS = 8;

const int m_l = 1U << (L_BITS-1);
const int m_t = 1U << (T_BITS-1);
const int m_y = 1U << (Y_BITS-1);
const int m_x = 1U << (X_BITS-1);
const vec4_t u_x = 1, u_y = u_x << X_BITS, u_t = u_y << Y_BITS, u_l = u_t << T_BITS;
const vec4_t L_MASK = 0 - u_l, T_MASK = u_l - u_t, Y_MASK = u_t - u_y, X_MASK = u_y - u_x;

class vec4 {
    vec4_t value;
public:
    constexpr vec4(int x, int y, int t, int l)
    {
        value = (l << (T_BITS + Y_BITS + X_BITS) & L_MASK)
              | (t << (Y_BITS + X_BITS) & T_MASK)
              | (y << X_BITS & Y_MASK)
              | (x & X_MASK);
    }
    constexpr int l() const
    {
        int w = value >> (T_BITS + Y_BITS + X_BITS);
        return (w ^ m_l) - m_l;
    }
    constexpr int t() const
    {
        int w = (value & T_MASK) >> (Y_BITS + X_BITS);
        return (w ^ m_t) - m_t;
    }
    constexpr int y() const
    {
        int w = (value & Y_MASK) >> Y_BITS;
        return (w ^ m_y) - m_y;
    }
    constexpr int x() const
    {
        int x = value & X_MASK;
        return (x ^ m_x) - m_x;
    }
    constexpr vec4 operator +(const vec4& v) const
    {
        vec4 result = v;
        result.value += value;
        vec4_t x = result.value ^ value ^ v.value;
        result.value -= x & (u_l | u_t | u_y | u_x);
        return result;
    }
    constexpr vec4 operator -() const
    {
        vec4 temp = *this + vec4(-1,-1,-1,-1);
        temp.value = ~temp.value;
        return temp;
    }
    constexpr vec4 operator -(const vec4& v) const
    {
        return *this + (-v);
    }
    constexpr vec4 operator *(const int& scalar) const
    {
        return vec4(scalar*x(), scalar*y(), scalar*t(), scalar*l());
    }
    constexpr bool operator ==(const vec4&) const = default;
    constexpr bool operator <(const vec4& other) const
    {
        return value < other.value;
    }
    friend std::ostream& operator<<(std::ostream& os, const vec4& v)
    {
        os << "0x" << std::hex << std::setw(8) << std::setfill('0') << v.value << std::dec << std::setfill(' ') << std::setw(0);
        os << "(" << v.x() << "," << v.y() << "," << v.t() << "," << v.l() << ")";
        return os;
    }
    std::string to_string() const
    {
        std::stringstream sstm;
        sstm << "0x" << std::hex << std::setw(8) << std::setfill('0') << value << std::dec << std::setfill(' ') << std::setw(0);
        sstm << "(" << x() << "," << y() << "," << t() << "," << l() << ")";
        return sstm.str();
    }
    bool outbound() const
    {
        return (~vec4(0x7,0x7,-1,-1).value & value);
    }
    int xy() const
    {
        const int mask_x = 0b111, mask_y = 0b111000, y_shift = X_BITS-3;
        return (value & mask_x) | (value >> y_shift & mask_y);
    }
};


#endif /* vec4_h */
