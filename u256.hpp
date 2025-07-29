#pragma once

#include <algorithm> // std::reverse
#include <tuple>     // std::ignore, std::tie
#include <utility>   // std::pair
#include <cassert>   // assert
#include <string>    // std::string

#include "u128.hpp"
#include "u128_utils.h"

namespace u256
{

    using ULOW = u128::U128; // Тип половинок: старшей и младшей частей составного числа.

    static constexpr auto INF = "inf";

    struct Quadrupole
    { // Структура для задания дроби (A*M + B) / (C*M + D).
        // M - множитель системы счисления, 2^W, W = 128 - битовая ширина половинок.
        ULOW A;
        ULOW B;
        ULOW C;
        ULOW D;
    };

    struct Signess
    { // Структура для задания знаков двух чисел.
        bool s1;
        bool s2;
    };

    struct Dipole
    { // Структура для задания числа (A*M + B).
        // M - множитель системы счисления, 2^W, W = 128 - битовая ширина половинок.
        ULOW A;
        ULOW B;
    };

    static constexpr char DIGITS[10]{'0', '1', '2', '3', '4',
                                     '5', '6', '7', '8', '9'};

    struct Singular
    {
        int mOverflow = 0;
        int mNaN = 0;
        constexpr explicit Singular() = default;
        constexpr Singular(bool is_overflow, bool is_nan) : mOverflow{is_overflow}, mNaN{is_nan} {};
        constexpr Singular(const Singular &other) = default;
        constexpr Singular(Singular &&other) = default;
        constexpr Singular &operator=(const Singular &other) = default;
        constexpr Singular &operator=(Singular &&other) = default;
        bool operator()() const { return mNaN != 0 || mOverflow != 0; }
        bool operator==(const Singular &other) const
        {
            return !(mOverflow || other.mOverflow || mNaN || other.mNaN);
        }
        auto operator<=>(const Singular &other) const = default;
        bool IsOverflow() const { return mOverflow != 0; }
        bool IsNaN() const { return mNaN != 0; }
    };

    struct Sign
    {
        int mSign = 0;
        constexpr explicit Sign() = default;
        constexpr Sign(const Sign &other) = default;
        constexpr Sign(Sign &&other) = default;
        constexpr Sign(bool value) : mSign{value} {};
        constexpr Sign &operator=(const Sign &other) = default;
        constexpr Sign &operator=(Sign &&other) = default;
        Sign &operator^(const Sign &other)
        {
            this->mSign = operator()() ^ other.operator()();
            return *this;
        }
        bool operator()() const { return mSign != 0; }
        void operator-()
        {
            mSign = 1 - operator()();
        }
        bool operator==(const Sign &other) const
        {
            return mSign == other.mSign ? true : ((operator()() && other.operator()()) || (!operator()() && !other.operator()()));
        }
        auto operator<=>(const Sign &other) = delete;
    };

    struct U256;

    U256 shl128(U256 x);

    // High/Low структура 256-битного числа со знаком и флагом переполнения.
    struct U256
    {
        // Битовая полуширина половинок.
        static constexpr int mHalfWidth = 128 / 2;
        // Наибольшее значение половинок, M-1.
        static constexpr ULOW mMaxULOW = ULOW::get_max_value();
        ULOW mLow {0};
        ULOW mHigh {0};
        Sign mSign{};
        Singular mSingular{};

        explicit constexpr U256() = default;

        explicit constexpr U256(uint64_t x)
            : mLow{x} {};

        explicit constexpr U256(ULOW low, ULOW high, Sign sign = false)
            : mLow{low}, mHigh{high}, mSign{sign} {};

        constexpr U256(const U256 &other) = default;

        constexpr U256(U256 &&other) = default;

        constexpr U256 &operator=(const U256 &other) = default;

        bool operator==(const U256 &other) const
        {
            const auto has_singular = mSingular != other.mSingular;
            return has_singular ? false : *this <=> other == 0;
        }

        std::partial_ordering operator<=>(const U256 &other) const
        {
            const auto has_singular = mSingular != other.mSingular;
            if (has_singular)
            {
                return std::partial_ordering::unordered;
            }
            const bool equal_signs = mSign() == other.mSign();
            if (equal_signs)
            {
                auto high_cmp = mSign() ? other.mHigh <=> mHigh : mHigh <=> other.mHigh;
                if (high_cmp != 0)
                {
                    return high_cmp;
                }
                return mSign() ? other.mLow <=> mLow : mLow <=> other.mLow;
            }
            else
            {
                if (mSign())
                {
                    auto high_cmp = mHigh <=> other.mHigh;
                    if (high_cmp != 0)
                    {
                        return (high_cmp > 0) ? other.mHigh <=> mHigh : high_cmp;
                    }
                    auto low_cmp = mLow <=> other.mLow;
                    if (low_cmp != 0)
                    {
                        return (low_cmp > 0) ? other.mLow <=> mLow : low_cmp;
                    }
                    else
                    {
                        return !mLow.is_zero() || !mHigh.is_zero() ? -1 <=> 1 : 1 <=> 1;
                    }
                }
                else
                {
                    auto high_cmp = mHigh <=> other.mHigh;
                    if (high_cmp != 0)
                    {
                        return (high_cmp < 0) ? other.mHigh <=> mHigh : high_cmp;
                    }
                    auto low_cmp = mLow <=> other.mLow;
                    if (low_cmp != 0)
                    {
                        return (low_cmp < 0) ? other.mLow <=> mLow : low_cmp;
                    }
                    else
                    {
                        return !mLow.is_zero() || !mHigh.is_zero() ? 1 <=> -1 : 1 <=> 1;
                    }
                }
            }
        }

        bool is_singular() const
        {
            return mSingular();
        }

        bool is_overflow() const
        {
            return mSingular.IsOverflow() && !mSingular.IsNaN();
        }

        bool is_nan() const
        {
            return mSingular.IsNaN() && !mSingular.IsOverflow();
        }

        bool is_zero() const
        {
            return mLow.is_zero() && mHigh.is_zero() && !is_singular();
        }

        bool is_unit() const
        {
            return mLow.is_unit() && mHigh.is_zero() && !mSign() && !is_singular();
        }

        bool is_negative() const
        {
            return !is_zero() && mSign() && !is_singular();
        }

        bool is_positive() const
        {
            return !is_zero() && !mSign() && !is_singular();
        }

        bool is_nonegative() const
        {
            return is_positive() || is_zero();
        }

        void set_overflow()
        {
            mSingular.mOverflow = 1;
            mSingular.mNaN = 0;
        }

        void set_nan()
        {
            mSingular.mOverflow = 0;
            mSingular.mNaN = 1;
        }

        U256 operator<<(const uint32_t shift) const
        {
            U256 result = *this;
            constexpr U256 two {2};
            for (uint32_t i = 0; i < shift; i++) {
                result = result * two;
            }
            return result;
        }

        U256 operator>>(const uint32_t shift) const
        {
            U256 result = *this;
            constexpr U256 two {2};
            for (uint32_t i = 0; i < shift; i++) {
                result = (result / two).first;
            }
            return result;
        }

        U256 operator&(const U256& mask) const
        {
            U256 result = *this;
            result.mLow &= mask.mLow;
            result.mHigh &= mask.mHigh;
            return result;
        }

        U256& operator&=(const U256& mask)
        {
            *this = *this & mask;
            return *this;
        }

        U256 operator-() const
        {
            U256 result = *this;
            -result.mSign;
            return result;
        }

        U256 abs() const
        {
            U256 result = *this;
            result.mSign = false;
            return result;
        }

        U256 operator+(U256 rhs) const
        {
            U256 result{};
            U256 X = *this;
            if (X.is_singular())
            {
                return X;
            }
            if (rhs.is_singular())
            {
                X.mSingular = rhs.mSingular;
                return X;
            }
            if (X.is_negative() && !rhs.is_negative())
            {
                X.mSign = false;
                result = rhs - X;
                return result;
            }
            if (!X.is_negative() && rhs.is_negative())
            {
                rhs.mSign = false;
                result = X - rhs;
                return result;
            }
            result.mLow = X.mLow + rhs.mLow;
            const ULOW c1 = result.mLow < u128::utils::min(X.mLow, rhs.mLow) ? ULOW{1} : ULOW{0};
            result.mHigh = X.mHigh + rhs.mHigh;
            const int c2 = result.mHigh < u128::utils::min(X.mHigh, rhs.mHigh);
            ULOW tmp = result.mHigh;
            result.mHigh = tmp + c1;
            const int c3 = result.mHigh < std::min(tmp, c1);
            result.mSingular.mOverflow = c2 || c3;
            if (X.mSign() && rhs.mSign())
            {
                result.mSign = true;
            }
            return result;
        }

        U256 &operator+=(U256 other)
        {
            *this = *this + other;
            return *this;
        }

        U256 operator-(U256 rhs) const
        {
            U256 result{};
            U256 X = *this;
            if (X.is_singular())
            {
                return X;
            }
            if (rhs.is_singular())
            {
                X.mSingular = rhs.mSingular;
                return X;
            }
            if (X.is_negative() && !rhs.is_negative())
            {
                rhs.mSign = 1;
                result = rhs + X;
                return result;
            }
            if (!X.is_negative() && rhs.is_negative())
            {
                rhs.mSign = false;
                result = X + rhs;
                return result;
            }
            if (X.is_negative() && rhs.is_negative())
            {
                rhs.mSign = false;
                X.mSign = false;
                result = rhs - X;
                return result;
            }
            if (X.is_zero())
            {
                result = rhs;
                -result.mSign;
                return result;
            }
            result.mLow = X.mLow - rhs.mLow;
            result.mHigh = X.mHigh - rhs.mHigh;
            const bool borrow = X.mLow < rhs.mLow;
            const bool hasUnit = X.mHigh > rhs.mHigh;
            if (borrow && hasUnit)
            {
                result.mHigh.dec();
            }
            if (borrow && !hasUnit)
            {
                result = rhs - X;
                -result.mSign;
                return result;
            }
            if (!borrow && X.mHigh < rhs.mHigh)
            {
                result.mHigh = -result.mHigh - ULOW(result.mLow.is_zero() ? 0 : 1);
                result.mLow = -result.mLow;
                result.mSign = true;
            }
            return result;
        }

        U256 &operator-=(U256 other)
        {
            *this = *this - other;
            return *this;
        }

        /**
         * @brief Инкремент числа.
         * @return Число + 1.
         */
        U256 &inc()
        {
            *this = *this + U256{1};
            return *this;
        }

        /**
         * @brief Декремент числа.
         * @return Число - 1.
         */
        U256 &dec()
        {
            *this = *this - U256{1};
            return *this;
        }

        U256 mult128(ULOW x, ULOW y) const
        {
            const ULOW MASK = (ULOW{1} << mHalfWidth) - ULOW{1};
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
            U256 result{t1, ULOW{0}};
            const ULOW div = (q + s) + ((p + r + t) >> mHalfWidth);
            const auto p1 = t21 << mHalfWidth;
            const auto p2 = t22 << mHalfWidth;
            const ULOW mod = p1 + p2;
            result.mLow += mod;
            result.mHigh += div;
            result.mHigh += t3;
            result.mSingular.mOverflow = result.mHigh < t3;
            return result;
        }

        U256 operator*(ULOW rhs) const
        {
            U256 result = mult128(mLow, rhs);
            U256 tmp = mult128(mHigh, rhs);
            const bool is_overflow = !tmp.mHigh.is_zero();
            tmp.mHigh = tmp.mLow;
            tmp.mLow = ULOW{0};
            result += tmp;
            result.mSign = !result.is_zero() ? this->mSign() : false;
            if (is_overflow)
                result.set_overflow();
            return result;
        }

        U256 operator*(U256 rhs) const
        {
            const U256 X = *this;
            if (X.is_overflow() || rhs.is_overflow())
            {
                U256 result;
                result.set_overflow();
                return result;
            }
            if (X.is_nan() || rhs.is_nan())
            {
                U256 result;
                result.set_nan();
                return result;
            }
            U256 result = X * rhs.mLow;
            if (result.is_singular())
            {
                return result;
            }
            result.mSign = this->mSign() ^ rhs.mSign();
            const auto tmp = X * rhs.mHigh;
            result = result + shl128(tmp);
            return result;
        }

        U256 div10() const
        { // Специальный метод деления на 10 для формирования
            // строкового представления числа.
            U256 X = *this;
            if (X.is_singular())
            {
                return X;
            }
            const bool sign = X.mSign();
            X.mSign = false;
            auto [Q, R] = X.mHigh / ULOW(10, 0);
            ULOW N = R * (mMaxULOW / ULOW(10, 0)).first + (X.mLow / ULOW(10, 0)).first;
            U256 result{};
            result.mHigh = Q;
            result.mLow = N;
            const U256 tmp = result * ULOW(10, 0);
            U256 E = X - tmp;
            while (!E.mHigh.is_zero() || E.mLow >= ULOW(10, 0))
            {
                std::tie(Q, R) = E.mHigh / ULOW(10, 0);
                N = R * (mMaxULOW / ULOW(10, 0)).first + (E.mLow / ULOW(10, 0)).first;
                U256 tmp{N, Q};
                result += tmp;
                E -= tmp * ULOW(10, 0);
            }
            result.mSign = sign;
            return result;
        }

        int mod10() const
        { // Специальный метод нахождения остатка от деления на 10 для формирования
            // строкового представления числа.
            if (this->is_singular())
            {
                return -1;
            }
            const int multiplier_mod10 = (mMaxULOW / 10).second.mLow + 1;
            return ((mLow / 10).second.mLow + multiplier_mod10 * (mHigh / 10).second.mLow) % 10;
        }

        // Метод итеративного деления широкого числа на узкое.
        // Наиболее вероятное количество итераций: ~N/4, где N - количество битов узкого числа.
        // В данном случае имеем ~64/4 = 16 итераций.
        // Максимум до ~(N+1) итерации.
        std::pair<U256, U256> operator/(ULOW y) const
        {
            assert(!y.is_zero());
            const U256 X = *this;
            if (X.is_singular())
            {
                return std::make_pair(X, U256{0});
            }
            auto [Q, R] = X.mHigh / y;
            ULOW N = R * (mMaxULOW / y).first + (X.mLow / y).first;
            U256 result{N, Q, X.mSign};
            U256 E = X - result * y; // Остаток от деления.
            for (;;)
            {
                std::tie(Q, R) = E.mHigh / y;
                N = R * (mMaxULOW / y).first + (E.mLow / y).first;
                U256 tmp{N, Q, E.mSign};
                if (tmp.is_zero())
                {
                    break;
                }
                result += tmp;
                E -= tmp * y;
            }
            if (E.is_negative())
            { // И при этом не равно нулю.
                result.dec();
                U256 tmp{y, ULOW{0}};
                E += tmp;
            }
            return std::make_pair(result, E);
        }

        std::pair<U256, U256> operator/=(ULOW y)
        {
            U256 remainder;
            std::tie(*this, remainder) = *this / y;
            return std::make_pair(*this, remainder);
        }

        // Метод деления двух широких чисел.
        // Отсутствует "раскачка" алгоритма для "плохих" случаев деления: (A*M + B)/(1*M + D).
        // Наиболее вероятное общее количество итераций: 4...6.
        std::pair<U256, U256> operator/(const U256 other) const
        {
            U256 X = *this;
            U256 Y = other;
            if (X.is_overflow() || Y.is_overflow())
            {
                U256 result;
                result.set_overflow();
                return std::make_pair(result, U256{0});
            }
            if (X.is_nan() || Y.is_nan())
            {
                U256 result;
                result.set_nan();
                return std::make_pair(result, U256{0});
            }
            if (Y.mHigh.is_zero())
            {
                X.mSign = X.mSign() ^ Y.mSign();
                auto result = X / Y.mLow;
                return result;
            }
            const bool make_sign_inverse = X.mSign != Y.mSign;
            X.mSign = make_sign_inverse;
            Y.mSign = 0;
            const auto& [Q, R] = X.mHigh / Y.mHigh;
            const ULOW Delta = mMaxULOW - Y.mLow;
            const U256 DeltaQ = mult128(Delta, Q);
            U256 W1 = U256{ULOW{0}, R} - U256{ULOW{0}, Q};
            W1 = W1 + DeltaQ;
            const ULOW C1 = (Y.mHigh < mMaxULOW) ? Y.mHigh + ULOW{1} : mMaxULOW;
            const ULOW W2 = mMaxULOW - (Delta / C1).first;
            auto [Quotient, _] = W1 / W2;
            std::tie(Quotient, std::ignore) = Quotient / C1;
            U256 result = U256{Q, ULOW{0}} + Quotient;
            if (make_sign_inverse)
            {
                result = -result;
            }
            U256 N = Y * result.mLow;
            if (make_sign_inverse)
            {
                N = -N;
            }
            assert(!N.is_overflow());
            U256 Error = X - N;
            U256 More = Error - Y;
            bool do_inc = More.is_nonegative();
            bool do_dec = Error.is_negative();
            while (do_dec || do_inc)
            {
                if (do_dec)
                {
                    result.dec();
                    Error += Y;
                }
                if (do_inc)
                {
                    result.inc();
                    Error -= Y;
                }
                More = Error - Y;
                do_inc = More.is_nonegative();
                do_dec = Error.is_negative();
            }
            return std::make_pair(result, Error);
        }

        /**
         * Возвращает строковое представление числа.
         */
        std::string value() const
        {
            std::string result{};
            if (this->is_overflow())
            {
                result = INF;
                return result;
            }
            if (this->is_nan())
            {
                result = "";
                return result;
            }
            U256 X = *this;
            while (!X.is_zero())
            {
                const int d = X.mod10();
                if (d < 0)
                {
                    return result;
                }
                result.push_back(DIGITS[d]);
                X = X.div10();
            }
            if (this->is_negative() && !this->is_zero())
            {
                result.push_back('-');
            }
            std::reverse(result.begin(), result.end());
            return result.length() != 0 ? result : "0";
        }

    }; // struct U256

    inline U256 shl128(U256 x)
    { // x * 2^128
        U256 result{ULOW{0}, x.mLow, x.mSign};
        result.mSingular = x.mSingular;
        if (!x.mHigh.is_zero() && !x.is_singular())
        {
            result.set_overflow();
        }
        return result;
    }

} // namespace u256
