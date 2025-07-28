#pragma once

#include "u128.hpp"
#include <map>        // std::map
#include <vector>     // std::vector
#include <tuple>      // std::ignore, std::tie
#include <utility>    // std::pair
#include <functional> // std::function
#include "solver.hpp" // GaussJordan

namespace u128
{
    namespace utils
    {
        inline U128 get_max_value()
        {
            U128 result{};
            result.mLow = -1;
            result.mHigh = -1;
            return result;
        }

        inline U128 int_power(ULOW x, int y)
        {
            u128::U128 result = get_unit();
            for (int i = 1; i <= y; ++i)
            {
                result = result * x;
            }
            return result;
        }

        /**
         * @brief Количество цифр числа.
         * @param x Число.
         * @return Количество цифр, минимум 1.
         */
        inline int num_of_digits(U128 x)
        {
            int i = 0;
            while (!x.is_zero())
            {
                x = x.div10();
                i++;
            }
            return i + (i == 0);
        }

        /**
         * НОД.
         */
        inline U128 gcd(U128 x, U128 y)
        {
            if (x.is_singular())
                return x;
            if (y.is_singular())
                return y;
            if (x == y)
            {
                return x;
            }
            if (x > y)
            {
                while (!y.is_zero())
                {
                    const U128 y_copy = y;
                    y = (x / y).second;
                    x = y_copy;
                }
                return x;
            }
            else
            {
                while (!x.is_zero())
                {
                    const U128 x_copy = x;
                    x = (y / x).second;
                    y = x_copy;
                }
                return y;
            }
        }

        /**
         * Целочисленный квадратный корень.
         * @param exact Точно ли прошло извлечение корня.
         */
        inline U128 isqrt(U128 x, bool &exact)
        {
            exact = false;
            if (x.is_singular())
            {
                return x;
            }
            const U128 c{ULOW(1) << U128::mHalfWidth, 0};
            U128 result;
            x = x.abs();
            if (x >= U128{0, 1})
            {
                result = c;
            }
            else
            {
                result = U128{ULOW(1) << (U128::mHalfWidth / 2), 0};
            }
            U128 prevprev = get_unit_neg();
            U128 prev = x;
            for (;;)
            {
                prevprev = prev;
                prev = result;
                const auto [tmp, remainder] = x / result;
                std::tie(result, std::ignore) = (result + tmp) / 2;
                if (result.is_zero())
                {
                    exact = true;
                    return result;
                }
                if (result == prev)
                {
                    exact = (tmp == prev) && remainder.is_zero(); // Нет остатка от деления.
                    return result;
                }
                if (result == prevprev)
                {
                    return prev;
                }
            }
        }

        /**
         * @brief Является ли заданное число квадратичным вычетом.
         * @param x Тестируемое число.
         * @param p Простой модуль.
         * @return Да/нет.
         */
        inline bool is_quadratiq_residue(U128 x, U128 p)
        {
            // y^2 = x mod p
            auto [_, r1] = x / p;
            for (U128 y = u128::get_zero(); y < p; y.inc())
            {
                U128 sq = y * y;
                auto [_, r2] = sq / p;
                if (r2 == r1)
                    return true;
            }
            return false;
        }

        /**
         * @brief Возвращает корень квадратный из заданного числа
         * по заданному модулю.
         * @param x Число.
         * @param p Простой модуль.
         * @return Два значения корня.
         */
        inline std::pair<U128, U128> sqrt_mod(U128 x, U128 p)
        {
            // return  sqrt(x) mod p
            U128 result[2];
            int idx = 0;
            const auto [_, r1] = x / p;
            for (U128 y = u128::get_zero(); y < p; y.inc())
            {
                U128 sq = y * y;
                auto [_, r2] = sq / p;
                if (r2 == r1)
                    result[idx++] = y;
            }
            if (idx == 1)
            { // Если не был установлен второй корень.
                result[1] = result[0];
            }
            return std::make_pair(result[0], result[1]);
        }

        inline bool is_prime(U128 x)
        {
            [[maybe_unused]] bool exact;
            const auto x_sqrt = isqrt(x, exact) + u128::get_unit();
            U128 d{2, 0};
            bool is_ok = true;
            while (d < x_sqrt)
            {
                auto [_, remainder] = x / d;
                is_ok &= !remainder.is_zero();
                d.inc();
            }
            return is_ok;
        }

        class PrimesGenerator
        {
        public:
            U128 next()
            {
                if (mPrimes.empty())
                {
                    mPrimes.push_back(U128{2, 0});
                    return mPrimes.back();
                }
                U128 last_prime = mPrimes.back();
                for (;;)
                {
                    bool is_prime = true;
                    last_prime.inc();
                    for (const auto &p : mPrimes)
                    {
                        U128 rem = (last_prime / p).second;
                        if (rem.is_zero())
                        {
                            is_prime = false;
                            break;
                        }
                    }
                    if (is_prime)
                    {
                        mPrimes.push_back(last_prime);
                        return mPrimes.back();
                    }
                }
            }

        private:
            std::vector<U128> mPrimes;
        };

        /**
         * @brief Делит первое число на второе до "упора".
         * @param x Делимое.
         * @param q Делитель.
         * @return Пара {Делитель, количество успешных делений}
         */
        inline std::pair<U128, int> div_by_q(U128 &x, ULOW q)
        {
            auto [tmp, remainder] = x / q;
            int i = 0;
            while (remainder.is_zero())
            {
                i++;
                x = tmp;
                std::tie(tmp, remainder) = x / q;
            }
            return std::make_pair(U128{q, 0}, i);
        }

        inline std::pair<U128, U128> ferma_method(U128 x)
        {
            U128 x_sqrt;
            {
                bool is_exact;
                x_sqrt = isqrt(x, is_exact);
                if (is_exact)
                    return std::make_pair(x_sqrt, x_sqrt);
            }
            const auto error = x - x_sqrt * x_sqrt;
            auto y = U128{2, 0} * x_sqrt + u128::get_unit() - error;
            {
                bool is_exact;
                auto y_sqrt = isqrt(y, is_exact);
                const auto delta = x_sqrt + x_sqrt + U128{3, 0};
                y = y + delta;
                if (is_exact)
                    return std::make_pair(x_sqrt + u128::get_unit() - y_sqrt, x_sqrt + u128::get_unit() + y_sqrt);
            }
            const auto &k_upper = x_sqrt;
            for (auto k = U128{2, 0};; k.inc())
            {
                if (k > k_upper)
                {
                    return std::make_pair(x, u128::get_unit()); // x - простое число.
                }
                if (k.mLow % 2)
                { // Проверка с другой стороны: ускоряет поиск.
                    // Основано на равенстве, следующем из метода Ферма: индекс k = (F^2 + x) / (2F) - floor(sqrt(x)).
                    // Здесь F - кандидат в множители, x - раскладываемое число.
                    const auto &N1 = k * k + x;
                    if ((N1.mLow % 2) == 0)
                    {
                        auto [q1, remainder] = N1 / (k + k); // Здесь k как некоторый множитель F.
                        if (remainder.is_zero() && (q1 > x_sqrt))
                        {
                            auto [q2, remainder] = x / k;
                            if (remainder.is_zero()) // На всякий случай оставим.
                                return std::make_pair(k, q2);
                        }
                    }
                }
                if (const auto &r = y.mod10(); (r != 1 && r != 9)) // Просеиваем заведомо лишние.
                    continue;
                bool is_exact;
                const auto y_sqrt = isqrt(y, is_exact);
                const auto delta = (x_sqrt + x_sqrt) + (k + k) + u128::get_unit();
                y = y + delta;
                if (!is_exact)
                    continue;
                const auto first_multiplier = x_sqrt + k - y_sqrt;
                return std::make_pair(first_multiplier, x_sqrt + k + y_sqrt);
            }
            return std::make_pair(x, u128::get_unit()); // По какой-то причине не раскладывается.
        };

        inline std::map<U128, int> factor(U128 x)
        {
            if (x.is_zero())
            {
                return {{x, 1}};
            }
            if (x == u128::get_unit())
            {
                return {{x, 1}};
            }
            if (x.is_singular())
            {
                return {{x, 1}};
            }
            x = x.abs();
            std::map<U128, int> result{};
            { // Обязательное для метода Ферма деление на 2.
                auto [p, i] = div_by_q(x, 2);
                if (i > 0)
                    result[p] = i;
                if (x < U128{2, 0})
                {
                    return result;
                }
            }
            // Делим на простые из списка: опционально.
            for (const auto &el : std::vector{3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41,
                                              43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127,
                                              131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
                                              223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293})
            {
                auto [p, i] = div_by_q(x, el);
                if (i > 0)
                    result[p] = i;
                if (x < U128{2, 0})
                {
                    return result;
                }
            }
            // Применяем метод Ферма рекурсивно.
            std::function<void(U128)> ferma_recursive;
            ferma_recursive = [&ferma_recursive, &result](U128 x) -> void
            {
                auto [a, b] = ferma_method(x);
                if (a == U128{1, 0})
                {
                    result[b]++;
                    return;
                }
                else if (b == U128{1, 0})
                {
                    result[a]++;
                    return;
                }
                ferma_recursive(a);
                ferma_recursive(b);
            };
            ferma_recursive(x);
            return result;
        }

        /**
         * @brief Разложить на простые множители методом квадратичного решета.
         * @param x Число.
         * @param sieve_size Размер решета, больше нуля.
         * @param factor_base Фактор-база (количество простых чисел-базисов), больше нуля.
         * @return Результат разложения.
         */
        inline std::map<U128, int> factor_qs(U128 x, unsigned int sieve_size, unsigned int factor_base)
        {
            std::map<U128, int> result{};
            if (sieve_size == 0 || factor_base == 0)
            {
                return result;
            }
            auto find_a_divisor = [sieve_size, factor_base](U128 x) -> U128
            {
                if (x.is_zero())
                    return x;
                if (x == u128::get_unit())
                    return x;
                std::vector<U128> base;
                PrimesGenerator pg;
                // std::cout << "Form factor base..." << std::flush;
                for (; base.size() < factor_base;)
                {
                    const U128 &p = pg.next();
                    if (is_quadratiq_residue(x, p))
                    {
                        base.push_back(p);
                        // std::cout << p.value() << ", ";
                    }
                }
                // std::cout << "Factor base: size: " << base.size() << std::endl;
                bool is_exact;
                U128 x_sqrt = isqrt(x, is_exact);
                if (!is_exact)
                {
                    x_sqrt.inc();
                }
                // std::cout << "Sqrt(x): " << x_sqrt.value() << std::endl;
                std::vector<U128> sieve(sieve_size);
                U128 ii{0, 0};
                // std::cout << "Sieve: \n";
                for (unsigned int i = 0; i < sieve.size(); ++i)
                {
                    sieve[i] = (ii + x_sqrt) * (ii + x_sqrt) - x;
                    // std::cout << sieve.at(i).value() << ", ";
                    ii.inc();
                }
                // std::cout << std::endl;
                std::vector<U128> sieve_original = sieve;
                for (const U128 &modulo : base)
                {
                    auto [root_1, root_2] = sqrt_mod(x, modulo);
                    // std::cout << "Sqrts pre: " << root_1.value() << ", " << root_2.value() << ", modulo: " << modulo.mLow << std::endl;
                    root_1 -= x_sqrt;
                    root_2 -= x_sqrt;
                    if (root_1.is_negative())
                    {
                        U128 delta_1 = (root_1.abs() / modulo).first;
                        root_1 += delta_1 * modulo;
                    }
                    if (root_1.is_negative())
                    {
                        root_1 += modulo;
                    }
                    if (root_2.is_negative())
                    {
                        U128 delta_2 = (root_2.abs() / modulo).first;
                        root_2 += delta_2 * modulo;
                    }
                    if (root_2.is_negative())
                    {
                        root_2 += modulo;
                    }
                    // std::cout << "Sqrts post: " << root_1.value() << ", " << root_2.value() << ", modulo: " << modulo.mLow << std::endl;
                    unsigned int idx = root_1.mLow;
                    while ((idx + 1) < sieve.size())
                    {
                        sieve[idx] = (sieve.at(idx) / modulo).first;
                        idx += modulo.mLow;
                    }
                    if (root_1 != root_2)
                    {
                        unsigned int idx = root_2.mLow;
                        while ((idx + 1) < sieve.size())
                        {
                            sieve[idx] = (sieve.at(idx) / modulo).first;
                            idx += modulo.mLow;
                        }
                    }
                }
                // std::cout << "Sieve after divisions: \n";
                std::vector<unsigned int> indices_where_unit_sieve;
                for (unsigned int i = 0; i < sieve.size(); ++i)
                {
                    // std::cout << sieve.at(i).value() << ", ";
                    if (sieve.at(i) == u128::get_unit())
                    {
                        indices_where_unit_sieve.push_back(i);
                    }
                }
                // std::cout << std::endl;
                // std::cout << "Indices: size: " << indices_where_unit_sieve.size() << std::endl;
                sieve.clear();
                std::vector<std::vector<int>> M;
                std::vector<U128> sieve_reduced;
                int idx = 0;
                for (const auto &index : indices_where_unit_sieve)
                {
                    M.push_back({});
                    const U128 &value = sieve_original.at(index);
                    sieve_reduced.push_back(value);
                    for (const U128 &modulo : base)
                    {
                        const auto &[_, r] = value / modulo;
                        M[idx].push_back(r.is_zero() ? 1 : 0);
                    }
                    idx++;
                }
                sieve_original.clear();
                // std::cout << "Matrix: \n";
                // for (const auto& v : M) {
                //     for (auto el : v) {
                //         std::cout << el << ", ";
                //     }
                //     std::cout << "; " << std::endl;
                // }
                // if (!M.empty())
                //     std::cout << "Matrix: rows: " << M.size() << ", cols: " << M.at(0).size() << std::endl;
                // else
                //     std::cout << "Empty matrix." << std::endl;
                // std::cout << "Try to solve..." << std::endl;
                const std::vector<std::set<int>> &solved_indices = solver::GaussJordan(M);
                M.clear();
                for (const auto &indices : solved_indices)
                {
                    // std::cout << "Solved indices: {";
                    // for (const auto& idx : indices) {
                    // std::cout << idx << ", ";
                    // }
                    // std::cout << "}." << std::endl;
                    U128 A = u128::get_unit();
                    std::map<U128, int> B_factors;
                    for (auto it = indices.begin(); it != indices.end(); it++)
                    {
                        const auto index = indices_where_unit_sieve.at(*it);
                        std::map<U128, int> sieve_factors;
                        {
                            const auto &val = sieve_reduced.at(*it);
                            // std::cout << "val: " << val.value() << ", index: " << index << std::endl;
                            for (const auto &modulo : base)
                            {
                                auto rem = (val / modulo).second;
                                // std::cout << "modulo: " << modulo.value() << ", rem: " << rem.value() << std::endl;
                                if (rem.is_zero())
                                {
                                    sieve_factors[modulo]++;
                                }
                            }
                        }
                        // std::cout << "A: " << A.value() << std::endl;
                        A = A * (x_sqrt + U128{index, 0});
                        for (const auto &[prime, power] : sieve_factors)
                        {
                            // std::cout << "prime: " << prime.value() << ", power: " << power << std::endl;
                            B_factors[prime] += power;
                        }
                        if (A.is_singular())
                        {
                            // std::cout << "Singularity was detected!\n";
                            continue;
                        }
                    }
                    for (auto &element : B_factors)
                    {
                        element.second /= 2;
                    }
                    U128 B = u128::get_unit();
                    for (const auto &[prime, power] : B_factors)
                    {
                        U128 tmp = u128::get_unit();
                        for (int i = 0; i < power; ++i)
                            tmp = tmp * prime;
                        B = B * tmp;
                    }
                    // std::cout << "B: " << B.value() << std::endl;
                    const U128 &C = A - B;
                    // std::cout << "ABC: " << A.value() << ", " << B.value() << ", " << C.value() << std::endl;
                    const U128 &GCD = gcd(C, x);
                    // std::cout << "GCD: " << GCD.value() << std::endl;
                    if (GCD < x && GCD > u128::get_unit())
                    {
                        // std::cout << "Fixed" << std::endl;
                        return GCD;
                    }
                }
                return x;
            }; // find_a_divisor()
            U128 y = u128::get_unit();
            for (;;)
            {
                // std::cout << "Try to find divisors: x, y: " << x.value() << ", " << y.value() << std::endl;
                const auto &divisor1 = find_a_divisor(x);
                const auto &divisor2 = find_a_divisor(y);
                if (divisor1 == u128::get_unit() && divisor2 == u128::get_unit())
                    break;
                // std::cout << "Divisors: " << divisor1.value() << ", " << divisor2.value() << std::endl;
                if (divisor2 == y && divisor2 != u128::get_unit())
                {
                    result[divisor2]++;
                }
                if (divisor1 == x && divisor1 != u128::get_unit())
                {
                    result[divisor1]++;
                    y = u128::get_unit();
                }
                else
                {
                    y = divisor1;
                }
                x = (x / divisor1).first;
            }
            return result;
        }

        inline U128 get_by_digit(int digit)
        {
            return U128{static_cast<u128::ULOW>(digit), 0};
        }

    }
}