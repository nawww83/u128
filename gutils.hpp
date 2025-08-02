#pragma once

namespace gutils {

template <typename T>
inline T min(T x, T y)
{
    if (x.is_singular()) return x;
    if (y.is_singular()) return y;
    if (x < y) return x; 
    else return y;
}

template <typename T>
inline T max(T x, T y)
{
    if (x.is_singular()) return x;
    if (y.is_singular()) return y;
    if (x > y) return x; 
    else return y;
}

}