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
constexpr vec4_t L_BITS = 8, T_BITS = 8, Y_BITS = 8, X_BITS = 8;

class vec4 {
    vec4_t value;
    
    constexpr vec4(vec4_t v) : value(v) {}
    constexpr static vec4_t m_l = 1U << (L_BITS-1);
    constexpr static vec4_t m_t = 1U << (T_BITS-1);
    constexpr static vec4_t m_y = 1U << (Y_BITS-1);
    constexpr static vec4_t m_x = 1U << (X_BITS-1);
    constexpr static vec4_t u_x = 1, u_y = u_x << X_BITS, u_t = u_y << Y_BITS, u_l = u_t << T_BITS;
    constexpr static vec4_t L_MASK = 0 - u_l, T_MASK = u_l - u_t, Y_MASK = u_t - u_y, X_MASK = u_y - u_x;
    constexpr static vec4_t mask_top = (u_x << (X_BITS-1)) | (u_y << (Y_BITS-1)) | (u_t << (T_BITS-1)) | (u_l << (L_BITS-1));
    constexpr static vec4_t mask_lower = ~mask_top;
public:
    constexpr vec4(int x, int y, int t, int l)
    {
        value = (static_cast<unsigned int>(l) << (T_BITS + Y_BITS + X_BITS) & L_MASK)
              | (static_cast<unsigned int>(t) << (Y_BITS + X_BITS) & T_MASK)
              | (static_cast<unsigned int>(y) << X_BITS & Y_MASK)
              | (static_cast<unsigned int>(x) & X_MASK);
    }
    constexpr int l() const
    {
        int w = static_cast<int>(value >> (T_BITS + Y_BITS + X_BITS));
        return (w ^ m_l) - m_l;
    }
    constexpr int t() const
    {
        int w = static_cast<int>((value & T_MASK) >> (Y_BITS + X_BITS));
        return (w ^ m_t) - m_t;
    }
    constexpr int y() const
    {
        int w = static_cast<int>((value & Y_MASK) >> Y_BITS);
        return (w ^ m_y) - m_y;
    }
    constexpr int x() const
    {
        int x = static_cast<int>(value & X_MASK);
        return (x ^ m_x) - m_x;
    }
    constexpr vec4 operator +(const vec4& v) const
    {
        vec4_t a = value, b = v.value;
        vec4_t sum = ((a & mask_lower) + (b & mask_lower)) ^ ((a^b) & mask_top);
        return vec4(sum);
    }
    constexpr vec4 operator -() const
    {
        vec4 temp = *this;
        temp.value = ~temp.value;
        return temp + vec4(1,1,1,1);
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
    constexpr auto operator <=>(const vec4& other) const
    {
        return value <=> other.value;
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
        constexpr int mask_x = 0b111, mask_y = 0b111000, y_shift = X_BITS-3;
        return (value & mask_x) | (value >> y_shift & mask_y);
    }
    vec4 tl() const
    {
        vec4 result = *this;
        result.value &= (L_MASK | T_MASK);
        return result;
    }
};


#endif /* vec4_h */
