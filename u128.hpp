#pragma once

#include <algorithm> // std::reverse
#include <cassert>
#include <cstdint>
#include <climits>
#include <string> // std::string

namespace u128 {

using ULOW = uint64_t; // Тип половинок: старшей и младшей частей составного числа.

static_assert(CHAR_BIT == 8);

struct Quadrupole { // Структура для задания дроби (A*M + B) / (C*M + D).
                    // M - множитель системы счисления, 2^W, W = 64 - битовая ширина половинок.
    ULOW A;
    ULOW B;
    ULOW C;
    ULOW D;
};

struct Signess { // Структура для задания знаков двух чисел.
    bool s1;
    bool s2;
};

static constexpr char DIGITS[10]{'0', '1', '2', '3', '4',
                                 '5', '6', '7', '8', '9'};

// High/Low структура 128-битного числа со знаком и флагом переполнения.
// Для иллюстрации алгоритма деления двух U128 чисел реализованы основные
// арифметические операторы, кроме умножения двух U128 чисел.
struct U128 {
    // Битовая полуширина половинок.
    static constexpr int mHalfWidth = (sizeof(ULOW) * CHAR_BIT) / 2;
    // Наибольшее значение половинок, M-1.
    static constexpr ULOW mMaxULOW = ULOW(-1);
    ULOW mHigh = 0;
    ULOW mLow = 0;
    bool mSign = 0;
    bool mOverflow = 0;

bool is_zero() const {
    return (mLow | mHigh | mOverflow) == 0;
}

bool is_negative() const {
    return mSign;
}

bool is_non_negative() const {
    return !mSign && !mOverflow;
}

bool is_nonzero_negative() const {
    return (mLow | mHigh) && mSign && !mOverflow;
}

bool is_overflow() const {
    return mOverflow;
}

constexpr static U128 get_zero() {
    return U128 {.mHigh = 0, .mLow = 0, .mSign = 0, .mOverflow = 0};
}

constexpr static U128 get_unit() {
    return U128 {.mHigh = 0, .mLow = 1, .mSign = 0, .mOverflow = 0};
}

constexpr static U128 get_unit_neg() {
    return U128 {.mHigh = 0, .mLow = 1, .mSign = 1, .mOverflow = 0};
}

U128 operator+(U128 rhs) const {
    U128 result{};
    U128 X = *this;
    if (X.is_negative() && !rhs.is_negative()) {
        X.mSign = 0;
        result = rhs - X;
        return result;
    }
    if (!X.is_negative() && rhs.is_negative()) {
        rhs.mSign = 0;
        result = X - rhs;
        return result;
    }
    result.mLow = X.mLow + rhs.mLow;
    const ULOW c1 = result.mLow < std::min(X.mLow, rhs.mLow);
    result.mHigh = X.mHigh + rhs.mHigh;
    const int c2 = result.mHigh < std::min(X.mHigh, rhs.mHigh);
    ULOW tmp = result.mHigh;
    result.mHigh = tmp + c1;
    const int c3 = result.mHigh < std::min(tmp, c1);
    result.mOverflow = c2 || c3;
    if (X.mSign && rhs.mSign) {
        result.mSign = 1;
    }
    return result;
}

U128& operator+=(U128 other) {
    *this = *this + other;
    return *this;
}

U128 operator-(U128 rhs) const {
    U128 result{};
    U128 X = *this;
    if (X.is_negative() && !rhs.is_negative()) {
        rhs.mSign = 1;
        result = rhs + X;
        return result;
    }
    if (!X.is_negative() && rhs.is_negative()) {
        rhs.mSign = 0;
        result = X + rhs;
        return result;
    }
    if (X.is_negative() && rhs.is_negative()) {
        rhs.mSign = 0;
        X.mSign = 0;
        result = rhs - X;
        return result;
    }
    if (X.is_zero()) {
        result = rhs;
        result.mSign = rhs.mSign ^ 1;
        return result;
    }
    result.mLow = X.mLow - rhs.mLow;
    result.mHigh = X.mHigh - rhs.mHigh;
    const bool borrow = X.mLow < rhs.mLow;
    const bool hasUnit = X.mHigh > rhs.mHigh;
    if (borrow && hasUnit) {
        result.mHigh -= ULOW(1);
    }
    if (borrow && !hasUnit) {
        result = rhs - X;
        result.mSign ^= 1;
        return result;
    }
    if (!borrow && X.mHigh < rhs.mHigh) {
        result.mHigh = -result.mHigh - ULOW(result.mLow != 0);
        result.mLow = -result.mLow;
        result.mSign = 1;
    }
    return result;
}

U128& operator-=(U128 other) {
    *this = *this - other;
    return *this;
}

U128 mult64(ULOW x, ULOW y) const {
    constexpr ULOW MASK = (ULOW(1) << mHalfWidth) - 1;
    const ULOW x_low = x & MASK;
    const ULOW y_low = y & MASK;
    const ULOW x_high = x >> mHalfWidth;
    const ULOW y_high = y >> mHalfWidth;
    const ULOW t1 = x_low * y_low;
    const ULOW t = t1 >> mHalfWidth;
    const ULOW t21 = x_low * y_high;
    const ULOW q = t21 >> mHalfWidth;
    const ULOW p = t21 & MASK;
    const ULOW t22 = x_high * y_low;
    const ULOW s = t22 >> mHalfWidth;
    const ULOW r = t22 & MASK;
    const ULOW t3 = x_high * y_high;
    U128 result{};
    result.mLow = t1;
    const ULOW div = (q + s) + ((p + r + t) >> mHalfWidth);
    const ULOW mod = (t21 << mHalfWidth) + (t22 << mHalfWidth);
    result.mLow += mod;
    result.mHigh += div;
    result.mHigh += t3;
    result.mOverflow = result.mHigh < t3 ? 1 : 0;
    return result;
}

U128 operator*(ULOW rhs) const {
    U128 result = mult64(mLow, rhs);
    U128 tmp = mult64(mHigh, rhs);
    tmp.mHigh = tmp.mLow;
    tmp.mLow = 0;
    result += tmp;
    result.mSign = !result.is_zero() ? this->mSign : 0;
    return result;
}

U128 div10() const { // Специальный метод деления на 10 для строкового представления числа.
    U128 X = *this;
    const int sign = X.mSign;
    X.mSign = 0;
    ULOW Q = X.mHigh / 10;
    ULOW R = X.mHigh % 10;
    ULOW N = R * (mMaxULOW / 10) + (X.mLow / 10);
    U128 result{.mHigh = Q, .mLow = N};
    U128 E = X - result * 10;
    while (E.mHigh != 0 || E.mLow >= 10) {
        Q = E.mHigh / 10;
        R = E.mHigh % 10;
        N = R * (mMaxULOW / 10) + (E.mLow / 10);
        const U128 tmp {.mHigh = Q, .mLow = N};
        result += tmp;
        E -= tmp * 10;
    }
    result.mSign = sign;
    return result;
}

// Метод итеративного деления широкого числа на узкое.
// Наиболее вероятное количество итераций: ~N/4, где N - количество битов узкого числа.
// В данном случае имеем ~64/4 = 16 итераций.
// Максимум до ~(N+1) итерации.
U128 operator/(ULOW y) const {
    assert(y != 0);
    const U128 X = *this;
    ULOW Q = X.mHigh / y;
    ULOW R = X.mHigh % y;
    ULOW N = R * (mMaxULOW / y) + (X.mLow / y);
    U128 result{.mHigh = Q, .mLow = N, .mSign = X.mSign};
    U128 E = X - result * y; // Ошибка от деления: остаток от деления.
    while (1) {
        Q = E.mHigh / y;
        R = E.mHigh % y;
        N = R * (mMaxULOW / y) + (E.mLow / y);
        const U128 tmp {.mHigh = Q, .mLow = N, .mSign = E.mSign};
        if (tmp.is_zero()) {
            break;
        }
        result += tmp;
        E -= tmp * y;
    }
    if (E.is_nonzero_negative()) {
        result -= get_unit();
        E += U128{.mHigh = 0, .mLow = y};
    }
    return result;
}

U128& operator/=(ULOW y) {
    *this = *this / y;
    return *this;
}

// Метод деления двух широких чисел.
// Отсутствует "раскачка" алгоритма для "плохих" случаев деления: (A*M + B)/(1*M + D).
// Наиболее вероятное общее количество итераций: 4...6.
U128 operator/(const U128 other) const {
    U128 X = *this;
    U128 Y = other;
    constexpr U128 ZERO = get_zero();
    constexpr U128 UNIT = get_unit();
    constexpr U128 UNIT_NEG = get_unit_neg();
    if (Y.mHigh == 0) {
        X.mSign ^= Y.mSign;
        U128 result = X / Y.mLow;
        return result;
    }
    const bool make_sign_inverse = X.mSign != Y.mSign;
    X.mSign = make_sign_inverse;
    Y.mSign = 0;
    const ULOW Q = X.mHigh / Y.mHigh;
    const ULOW R = X.mHigh % Y.mHigh;
    const ULOW Delta = mMaxULOW - Y.mLow;
    const U128 DeltaQ = mult64(Delta, Q);
    U128 W1 = U128{.mHigh = R, .mLow = 0} - U128{.mHigh = Q, .mLow = 0};
    W1 = W1 + DeltaQ;
    const ULOW C1 = (Y.mHigh < mMaxULOW) ? Y.mHigh + ULOW(1) : mMaxULOW;
    const ULOW W2 = mMaxULOW - Delta / C1;
    U128 Quotient = W1 / W2;
    Quotient = Quotient / C1;
    U128 result = U128{.mHigh = 0, .mLow = Q} + Quotient;
    result.mSign ^= make_sign_inverse;
    U128 N = Y * result.mLow;
    N.mSign ^= make_sign_inverse;
    assert(!N.is_overflow());
    U128 Error = X - N;
    U128 More = Error - Y;
    bool do_inc = More.is_non_negative();
    bool do_dec = Error.is_nonzero_negative();
    while (do_dec || do_inc) {
        result += (do_inc ? UNIT : (do_dec ? UNIT_NEG : ZERO));
        if (do_dec) {
            Error += Y;
        }
        if (do_inc) {
            Error -= Y;
        }
        More = Error - Y;
        do_inc = More.is_non_negative();
        do_dec = Error.is_nonzero_negative();
    }
    return result;
}

/**
 * Возвращает строковое представление числа.
 */
std::string value() const {
    std::string result{};
    if (this->is_overflow()) {
        result = "Overflow";
        return result;
    }
    U128 X = *this;
    constexpr int multiplier_mod10 = mMaxULOW % 10 + 1;
    while (!X.is_zero()) {
        const int d =
            ((X.mLow % 10) + multiplier_mod10 * (X.mHigh % 10)) % 10;
        result.push_back(DIGITS[d]);
        X = X.div10();
    }
    if (this->is_negative() && !this->is_zero()) {
        result.push_back('-');
    }
    std::reverse(result.begin(), result.end());
    return result.length() != 0 ? result : "0";
}

}; // struct U128

} // namespace u128