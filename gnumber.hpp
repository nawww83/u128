#pragma once

#include "singular.hpp"
#include "sign.hpp"
#include "gutils.hpp"

/**
 * Число разрядности 4*mHalfWitdh, где mHalfWidth = 64, 128, 256, ...
 * Базисом ULOW является число U128. На основании его можно сконструировать
 * 256-битное число как GNumber<U128, 64>, затем на основании 256-битного 512-битное и т.д.
 */
template <typename ULOW, unsigned int mHalfWidth>
struct GNumber
{
    static constexpr ULOW mMaxULOW = ULOW::get_max_value();
    ULOW mLow{0};
    ULOW mHigh{0};
    Sign mSign{};
    Singular mSingular{};

    explicit constexpr GNumber() = default;

    explicit constexpr GNumber(uint64_t x)
        : mLow{x} {};

    explicit constexpr GNumber(ULOW low, ULOW high, Sign sign = Sign{})
        : mLow{low}, mHigh{high}, mSign{sign} {};

    constexpr GNumber(const GNumber &other) = default;

    constexpr GNumber(GNumber &&other) = default;

    constexpr GNumber &operator=(const GNumber &other) = default;

    bool operator==(const GNumber &other) const
    {
        const auto has_singular = mSingular != other.mSingular;
        return has_singular ? false : *this <=> other == 0;
    }

    std::partial_ordering operator<=>(const GNumber &other) const
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

    /**
     * @brief Оператор сдвига влево.
     * @details Сохраняет знак. Поведение аналогично поведению
     * такого же оператора для встроенных в язык С++ беззнаковых чисел.
     */
    GNumber operator<<(uint32_t shift) const
    {
        GNumber result = *this;
        int ishift = shift % (mHalfWidth * 4);
        if (ishift < (mHalfWidth * 2))
        {
            const ULOW L = result.mLow >> ((mHalfWidth * 2) - ishift);
            result.mLow <<= ishift;
            result.mHigh <<= ishift;
            result.mHigh |= L;
        }
        else
        {
            result.mHigh = result.mLow;
            result.mLow = ULOW{0};
            ishift -= (mHalfWidth * 2);
            const ULOW L = result.mLow >> ((mHalfWidth * 2) - ishift);
            result.mLow <<= ishift;
            result.mHigh <<= ishift;
            result.mHigh |= L;
        }
        return result;
    }

    GNumber &operator<<=(const uint32_t shift)
    {
        *this = *this << shift;
        return *this;
    }

    /**
     * @brief Оператор сдвига вправо.
     * @details Сохраняет знак. Поведение аналогично поведению
     * такого же оператора для встроенных в язык С++ беззнаковых чисел.
     */
    GNumber operator>>(uint32_t shift) const
    {
        GNumber result = *this;
        int ishift = shift % (mHalfWidth * 4u);
        if (ishift < (mHalfWidth * 2))
        {
            ULOW mask = ~ULOW{0};
            mask <<= ishift;
            mask = ~mask;
            const ULOW H = result.mHigh & mask;
            result.mLow >>= ishift;
            result.mHigh >>= ishift;
            result.mLow |= H << ((mHalfWidth * 2) - ishift);
        }
        else
        {
            result.mLow = result.mHigh;
            result.mHigh = ULOW{0};
            ishift -= (mHalfWidth * 2);
            result.mLow >>= ishift;
        }
        return result;
    }

    GNumber &operator>>=(const uint32_t shift)
    {
        *this = *this >> shift;
        return *this;
    }

    GNumber operator&(const GNumber &mask) const
    {
        GNumber result = *this;
        result.mLow &= mask.mLow;
        result.mHigh &= mask.mHigh;
        return result;
    }

    GNumber &operator&=(const GNumber &mask)
    {
        *this = *this & mask;
        return *this;
    }

    GNumber operator|(const GNumber &mask) const
    {
        GNumber result = *this;
        result.mLow |= mask.mLow;
        result.mHigh |= mask.mHigh;
        return result;
    }

    GNumber &operator|=(const GNumber &mask)
    {
        *this = *this | mask;
        return *this;
    }

    /**
     * Оператор смены знака числа.
     */
    GNumber operator-() const
    {
        GNumber result = *this;
        -result.mSign;
        return result;
    }

    /**
     * Оператор инверсии битов. Не влияет на знак.
     */
    constexpr GNumber operator~() const
    {
        GNumber result = *this;
        result.mLow = ~result.mLow;
        result.mHigh = ~result.mHigh;
        return result;
    }

    GNumber abs() const
    {
        GNumber result = *this;
        result.mSign = false;
        return result;
    }

    GNumber operator+(GNumber rhs) const
    {
        GNumber result;
        GNumber X = *this;
        if (X.is_singular())
            return X;
        if (rhs.is_singular())
            return rhs;
        if (rhs.is_zero())
            return X;
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
        result.mLow = ULOW::add_mod(X.mLow, rhs.mLow);
        const ULOW c1 = result.mLow < gutils::min(X.mLow, rhs.mLow) ? ULOW{1} : ULOW{0};
        result.mHigh = ULOW::add_mod(X.mHigh, rhs.mHigh);
        const int c2 = result.mHigh < gutils::min(X.mHigh, rhs.mHigh);
        const ULOW tmp = result.mHigh;
        result.mHigh = ULOW::add_mod(tmp, c1);
        const int c3 = result.mHigh < gutils::min(tmp, c1);
        result.mSingular.mOverflow = c2 || c3;
        if (X.mSign() && rhs.mSign())
            result.mSign = true;
        return result;
    }

    GNumber &operator+=(const GNumber &other)
    {
        *this = *this + other;
        return *this;
    }

    GNumber operator-(GNumber rhs) const
    {
        GNumber result;
        GNumber X = *this;
        if (X.is_singular())
            return X;
        if (rhs.is_singular())
            return rhs;
        if (X.is_negative() && !rhs.is_negative())
        {
            rhs.mSign = true;
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
            return -rhs;
        result.mLow = ULOW::sub_mod(X.mLow, rhs.mLow);
        result.mHigh = ULOW::sub_mod(X.mHigh, rhs.mHigh);
        const bool borrow = X.mLow < rhs.mLow;
        const bool hasUnit = X.mHigh > rhs.mHigh;
        if (borrow && hasUnit)
            result.mHigh = ULOW::sub_mod(result.mHigh, ULOW{1});
        if (borrow && !hasUnit)
        {
            result = rhs - X;
            return -result;
        }
        if (!borrow && X.mHigh < rhs.mHigh)
        {
            result.mHigh = ULOW::sub_mod(ULOW::neg_mod(result.mHigh), ULOW{result.mLow.is_zero() ? 0ull : 1ull});
            result.mLow = ULOW::neg_mod(result.mLow);
            result.mSign = true;
        }
        return result;
    }

    GNumber &operator-=(const GNumber &other)
    {
        *this = *this - other;
        return *this;
    }

    /**
     * @brief Инкремент числа.
     * @return Число + 1.
     */
    GNumber &inc()
    {
        *this = *this + GNumber{1};
        return *this;
    }

    /**
     * @brief Декремент числа.
     * @return Число - 1.
     */
    GNumber &dec()
    {
        *this = *this - GNumber{1};
        return *this;
    }

    /**
     * Умножение двух "половинок" с расширением до полного числа.
     */
    static GNumber mult_ext(const ULOW &x, const ULOW &y)
    {
        const ULOW MASK = (ULOW{1} << mHalfWidth) - ULOW{1};
        const ULOW x_low = x & MASK;
        const ULOW y_low = y & MASK;
        const ULOW x_high = x >> mHalfWidth;
        const ULOW y_high = y >> mHalfWidth;
        const ULOW t1 = ULOW::mult_mod(x_low, y_low);
        const ULOW t = t1 >> mHalfWidth;
        const ULOW t21 = ULOW::mult_mod(x_low, y_high);
        const ULOW q = t21 >> mHalfWidth;
        const ULOW p = t21 & MASK;
        const ULOW t22 = ULOW::mult_mod(x_high, y_low);
        const ULOW s = t22 >> mHalfWidth;
        const ULOW r = t22 & MASK;
        const ULOW t3 = ULOW::mult_mod(x_high, y_high);
        GNumber result{t1, ULOW{0}};
        const ULOW div = ULOW::add_mod(ULOW::add_mod(q, s), (ULOW::add_mod(ULOW::add_mod(p, r), t) >> mHalfWidth));
        const auto p1 = t21 << mHalfWidth;
        const auto p2 = t22 << mHalfWidth;
        const ULOW mod = ULOW::add_mod(p1, p2);
        result.mLow = ULOW::add_mod(result.mLow, mod);
        result.mHigh = ULOW::add_mod(result.mHigh, div);
        result.mHigh = ULOW::add_mod(result.mHigh, t3);
        result.mSingular.mOverflow = result.mHigh < t3;
        return result;
    }

    /**
     * @brief Складывает два числа как беззнаковые по базовому модулю.
     * @details Поведение аналогично сложению встроенных в язык С++ беззнаковых чисел.
     */
    static GNumber add_mod(const GNumber &x, const GNumber &y)
    {
        if (x.is_overflow() || y.is_overflow())
        {
            GNumber result;
            result.set_overflow();
            return result;
        }
        if (x.is_nan() || y.is_nan())
        {
            GNumber result;
            result.set_nan();
            return result;
        }
        const ULOW &ac = ULOW::add_mod(x.mLow, y.mLow);
        ULOW bd = ULOW::add_mod(x.mHigh, y.mHigh);
        bd = ac < std::min(x.mLow, y.mLow) ? ULOW::add_mod(bd, ULOW{1u}) : bd;
        GNumber result{ac, bd};
        return result;
    }

    /**
     * @brief Вычитает два числа как беззнаковые по базовому модулю.
     * @details Поведение аналогично вычитанию встроенных в язык С++ беззнаковых чисел.
     */
    static GNumber sub_mod(const GNumber &x, const GNumber &y)
    {
        if (x.is_overflow() || y.is_overflow())
        {
            GNumber result;
            result.set_overflow();
            return result;
        }
        if (x.is_nan() || y.is_nan())
        {
            GNumber result;
            result.set_nan();
            return result;
        }
        if (x >= y)
        {
            const ULOW &ac = ULOW::sub_mod(x.mLow, y.mLow);
            ULOW bd = ULOW::sub_mod(x.mHigh, y.mHigh);
            bd = x.mLow < y.mLow ? ULOW::sub_mod(bd, ULOW{1u}) : bd;
            GNumber result{ac, bd};
            return result;
        }
        else
        {
            const GNumber &tmp = get_max_value() - y;
            return add_mod(x, add_mod(tmp, GNumber{1}));
        }
    }

    /**
     * @brief Смена знака, приводится по базовому модулю.
     * @details y = (-x) mod 2^W.
     */
    static GNumber neg_mod(const GNumber &x)
    {
        return sub_mod(GNumber{0}, x);
    }

    /**
     * @brief Вычисляет произведение двух W-битных чисел как беззнаковых по модулю 2^W.
     * @details Поведение аналогично умножению встроенных в язык С++ беззнаковых чисел.
     */
    static GNumber mult_mod(const GNumber &x, const GNumber &y)
    {
        // x*y = (a + w*b)(c + w*d) = ac + w*(ad + bc) + w*w*bd = (ac + w*(ad + bc)) mod 2^128;
        if (x.is_overflow() || y.is_overflow())
        {
            GNumber result;
            result.set_overflow();
            return result;
        }
        if (x.is_nan() || y.is_nan())
        {
            GNumber result;
            result.set_nan();
            return result;
        }
        const GNumber &ac = mult_ext(x.mLow, y.mLow);
        const GNumber &ad = mult_ext(x.mLow, y.mHigh);
        const GNumber &bc = mult_ext(x.mHigh, y.mLow);
        GNumber result = add_mod(ad, bc);
        result = shl_half_width_mod(result);
        result = add_mod(result, ac);
        return result;
    }

    GNumber operator*(const ULOW &rhs) const
    {
        if (this->is_singular())
            return *this;
        if (rhs.is_singular()) {
            GNumber result = *this;
            result.mSingular = rhs.mSingular;
            return result;
        }
        if (rhs.is_zero())
            return GNumber{0};
        GNumber result = mult_ext(mLow, rhs);
        GNumber tmp = mult_ext(mHigh, rhs);
        const bool is_overflow = !tmp.mHigh.is_zero();
        tmp.mHigh = tmp.mLow;
        tmp.mLow = ULOW{0};
        result += tmp;
        result.mSign = !result.is_zero() ? this->mSign() : false;
        if (is_overflow)
            result.set_overflow();
        return result;
    }

    GNumber operator*(const GNumber &rhs) const
    {
        const GNumber X = *this;
        if (X.is_overflow() || rhs.is_overflow())
        {
            GNumber result;
            result.set_overflow();
            return result;
        }
        if (X.is_nan() || rhs.is_nan())
        {
            GNumber result;
            result.set_nan();
            return result;
        }
        if (rhs.is_zero())
            return GNumber{0};
        GNumber result = X * rhs.mLow;
        if (result.is_singular())
            return result;
        result.mSign = this->mSign() ^ rhs.mSign();
        result = result + shl_half_width(X * rhs.mHigh);
        return result;
    }

    /**
     * Вспомогательный метод деления на 10 для формирования
     * строкового представления числа.
     */
    GNumber div10() const
    {
        GNumber X = *this;
        if (X.is_singular())
            return X;
        const bool sign = X.mSign();
        X.mSign = false;
        ULOW Q {X.mHigh.div10()};
        ULOW R {static_cast<unsigned int>(X.mHigh.mod10())};
        ULOW N { R * mMaxULOW.div10() + X.mLow.div10() };
        static constexpr ULOW TEN = ULOW{10};
        GNumber result{N, Q};
        GNumber E { X - (result * TEN) };
        while (!E.mHigh.is_zero() || E.mLow >= TEN)
        {
            Q = ULOW{E.mHigh.div10()};
            R = ULOW{static_cast<unsigned int>(E.mHigh.mod10())};
            N = R * mMaxULOW.div10() + E.mLow.div10();
            const GNumber tmp{N, Q};
            result += tmp;
            E -= tmp * TEN;
        }
        result.mSign = sign;
        return result;
    }

    /**
     * Вспомогательный метод нахождения остатка от деления на 10 для формирования
     * строкового представления числа.
     */
    int mod10() const
    {
        if (this->is_singular())
            return -1;
        const int multiplier_mod10 = mMaxULOW.mod10() + 1;
        return (mLow.mod10() + multiplier_mod10 * mHigh.mod10()) % 10;
    }

    // Метод итеративного деления широкого числа на узкое.
    // Наиболее вероятное количество итераций: ~N/4, где N - количество битов узкого числа.
    // В данном случае имеем ~64/4 = 16 итераций.
    // Максимум до ~(N+1) итерации.
    std::pair<GNumber, GNumber> operator/(const ULOW &y) const
    {
        assert(!y.is_zero());
        const GNumber X = *this;
        if (X.is_singular())
            return std::make_pair(X, GNumber{0});
        if (X.is_zero())
            return std::make_pair(GNumber{0}, GNumber{0});
        if (y == ULOW{1})
            return std::make_pair(X, GNumber{0});
        if (y == -ULOW{1})
            return std::make_pair(-X, GNumber{0});
        if (X.mHigh.is_zero() && X.mLow == y)
        {
            Sign sign = X.mSign() ^ y.mSign();
            return std::make_pair(GNumber{ULOW{1}, ULOW{0}, sign}, GNumber{0});
        }
        auto [Q, R] = X.mHigh / y;
        ULOW N { R * (mMaxULOW / y).first + (X.mLow / y).first };
        GNumber result{N, Q, X.mSign};
        GNumber E { X - result * y }; // Остаток от деления.
        for (;;)
        {
            std::tie(Q, R) = E.mHigh / y;
            N = R * (mMaxULOW / y).first + (E.mLow / y).first;
            GNumber tmp{N, Q, E.mSign};
            if (tmp.is_zero())
                break;
            result += tmp;
            E -= tmp * y;
        }
        if (E.is_negative()) // И при этом не равно нулю.
        {
            result.dec();
            GNumber tmp{y, ULOW{0}};
            E += tmp;
        }
        return std::make_pair(result, E);
    }

    std::pair<GNumber, GNumber> operator/=(const ULOW &y)
    {
        GNumber remainder;
        std::tie(*this, remainder) = *this / y;
        return std::make_pair(*this, remainder);
    }

    // Метод деления двух широких чисел.
    // Отсутствует "раскачка" алгоритма для "плохих" случаев деления: (A*M + B)/(1*M + D).
    // Наиболее вероятное общее количество итераций: 4...6.
    std::pair<GNumber, GNumber> operator/(const GNumber &other) const
    {
        assert(!other.is_zero());
        GNumber X = *this;
        GNumber Y = other;
        if (X.is_overflow() || Y.is_overflow())
        {
            GNumber result;
            result.set_overflow();
            return std::make_pair(result, GNumber{0});
        }
        if (X.is_nan() || Y.is_nan())
        {
            GNumber result;
            result.set_nan();
            return std::make_pair(result, GNumber{0});
        }
        if (X.is_zero())
            return std::make_pair(GNumber{0}, GNumber{0});
        if (X == Y)
            return std::make_pair(GNumber{1}, GNumber{0});
        if (X == -Y)
            return std::make_pair(-GNumber{1}, GNumber{0});
        if (Y == GNumber{1})
            return std::make_pair(X, GNumber{0});
        if (Y == -GNumber{1})
            return std::make_pair(-X, GNumber{0});
        if (Y.mHigh.is_zero())
        {
            X.mSign = X.mSign() ^ Y.mSign();
            return X / Y.mLow;
        }
        assert(X.mLow.is_nonegative());
        assert(Y.mLow.is_nonegative());
        assert(X.mHigh.is_nonegative());
        assert(Y.mHigh.is_nonegative());
        const bool make_sign_inverse = X.mSign != Y.mSign;
        X.mSign = make_sign_inverse;
        Y.mSign = false;
        const auto &[Q, R] = X.mHigh / Y.mHigh;
        const ULOW &Delta = mMaxULOW - Y.mLow;
        const GNumber &DeltaQ = mult_ext(Delta, Q);
        GNumber W1 { GNumber{ULOW{0}, R} - GNumber{ULOW{0}, Q} };
        W1 += DeltaQ;
        const ULOW &C1 = Y.mHigh < mMaxULOW ? Y.mHigh + ULOW{1} : mMaxULOW; // инкремент с насыщением.
        const ULOW &W2 = mMaxULOW - (Delta / C1).first;
        auto [Quotient, _] = W1 / W2;
        std::tie(Quotient, std::ignore) = Quotient / C1;
        GNumber result {GNumber{Q, ULOW{0}} + Quotient};
        GNumber N {Y * result.mLow};
        assert(!N.is_overflow());
        if (make_sign_inverse) {
            result = -result;
            N = -N;
        }
        GNumber Error {X - N};
        GNumber More {Error - Y};
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
        std::string result;
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
        GNumber X = *this;
        while (!X.is_zero())
        {
            const int d = X.mod10();
            if (d < 0)
                return result;
            result.push_back(DIGITS[d]);
            X = X.div10();
        }
        if (this->is_negative() && !this->is_zero())
            result.push_back('-');
        std::reverse(result.begin(), result.end());
        return result.length() != 0 ? result : "0";
    }

    static constexpr GNumber get_max_value()
    {
        GNumber result{ULOW{0}, ULOW{0}};
        return ~result;
    }

    /**
     * Сдвиг влево на полширины беззнаковой части по базовому модулю.
     * Сохраняет знак.
     */
    static GNumber shl_half_width_mod(const GNumber &x)
    { // sgn(x) * ((|x| * 2^(W/2)) mod 2^W)
        GNumber result{x.mHigh >> 1, x.mLow, x.mSign};
        result.mSingular = x.mSingular;
        return result;
    }

    /**
     * Сдвиг влево на полширины беззнаковой части.
     * Сохраняет знак. С переполнением.
     */
    static GNumber shl_half_width(const GNumber &x)
    { // x * 2^(W/2)
        GNumber result{ULOW{0}, x.mLow, x.mSign};
        result.mSingular = x.mSingular;
        if (!x.mHigh.is_zero() && !x.is_singular())
            result.set_overflow();
        return result;
    }
};