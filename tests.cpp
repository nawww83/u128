#include <iostream>
#include <random>
#include <cassert>
#include "tests.hpp"
#include "solver.hpp"

#include "gnumber.hpp"

#include "u128_utils.h"

using U256 = GNumber<U128, 64>;
using U512 = GNumber<U256, 128>;

static auto const seed = std::random_device{}();

static auto const internal_step = 1ll << 20;

/***
 * Генератор случайных чисел.
 */
auto roll_ulow = [urbg = std::mt19937{seed},
                  distr = std::uniform_int_distribution<ULOW>{}]() mutable -> ULOW
{
    return distr(urbg);
};

auto roll_uint = [urbg = std::mt19937{seed},
                  distr = std::uniform_int_distribution<uint>{}]() mutable -> uint
{
    return distr(urbg);
};

auto roll_bool = [urbg = std::mt19937{seed},
                  distr = std::uniform_int_distribution<uint>{}]() mutable -> bool
{
    return distr(urbg) % 2;
};

/**
 * Конструирует два 128-битных числа {B + M*A, D + M*C}.
 */
static std::pair<U128, U128> construct_two_128bit_numbers(const Quadrupole &q, const Signess &s)
{
    return std::make_pair(U128{q.B, q.A, Sign{s.s1}},
                          U128{q.D, q.C, Sign{s.s2}});
}

static std::pair<U256, U256> construct_two_256bit_numbers(const Quadrupole &q1, const Quadrupole &q2, const Signess &s)
{
    const auto &[N1, D1] = construct_two_128bit_numbers(q1, Signess{false});
    const auto &[N2, D2] = construct_two_128bit_numbers(q2, Signess{false});
    return std::make_pair(U256{N1, N2, Sign{s.s1}},
                          U256{D1, D2, Sign{s.s2}});
}

static std::pair<U512, U512> construct_two_512bit_numbers(const Quadrupole &q1, const Quadrupole &q2, const Quadrupole &q3, const Quadrupole &q4, const Signess &s)
{
    const auto &[N1, D1] = construct_two_256bit_numbers(q1, q2, Signess{false});
    const auto &[N2, D2] = construct_two_256bit_numbers(q3, q4, Signess{false});
    return std::make_pair(U512{N1, N2, Sign{s.s1}},
                          U512{D1, D2, Sign{s.s2}});
}

template <typename T>
PythonCaller<T>::PythonCaller()
{
    Py_Initialize();
    mMain = PyImport_AddModule("__main__");
    mGlobalDictionary = PyModule_GetDict(mMain);
    mLocalDictionary = PyDict_New();
}

template <typename T>
PythonCaller<T>::~PythonCaller()
{
    Py_Finalize();
}

template <typename T>
PyObject *PythonCaller<T>::Divide(T X, T Y) const
{
    const char *pythonScript = "quotient = nominator // denominator\n";
    PyDict_SetItemString(mLocalDictionary, "nominator", PyLong_FromString(X.value().c_str(), nullptr, 10));
    PyDict_SetItemString(mLocalDictionary, "denominator", PyLong_FromString(Y.value().c_str(), nullptr, 10));
    PyRun_String(pythonScript, Py_file_input, mGlobalDictionary, mLocalDictionary);
    return PyDict_GetItemString(mLocalDictionary, "quotient");
}

template <typename T>
PyObject *PythonCaller<T>::ISqrt(T X) const
{
    const char *pythonScript = "import math;y = math.isqrt(x)\n";
    PyDict_SetItemString(mLocalDictionary, "x", PyLong_FromString(X.value().c_str(), nullptr, 10));
    PyRun_String(pythonScript, Py_file_input, mGlobalDictionary, mLocalDictionary);
    return PyDict_GetItemString(mLocalDictionary, "y");
}

template <typename T>
bool PythonCaller<T>::Compare(PyObject *result, const char *reference) const
{
    PyObject *repr = PyObject_Repr(result);
    PyObject *str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
    const char *bytes = PyBytes_AsString(str);
    const bool is_ok = strcmp(bytes, reference) == 0;
    if (!is_ok)
    {
        printf("Python: %s\n", bytes);
        printf("C++: %s\n", reference);
    }
    Py_XDECREF(repr);
    Py_XDECREF(str);
    return is_ok;
}

template <typename T>
bool test_div(const std::pair<T, T> &z, PythonCaller<T> &caller)
{
    const auto &[q, _] = z.first / z.second;
    PyObject *quotient = caller.Divide(z.first, z.second);
    return caller.Compare(quotient, q.value().c_str());
}

bool test_isqrt(U128 z, PythonCaller<U128> &caller)
{
    bool exact;
    const U128 zi = u128::utils::isqrt(z, exact);
    const bool is_ok = exact == ((zi * zi) == z);
    PyObject *zs = caller.ISqrt(z);
    return caller.Compare(zs, zi.value().c_str()) && is_ok;
}

void test_isqrt_semi_randomly(long long N)
{
    if (N < 1)
    {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller<U128> caller;
    const std::vector<ULOW> choice{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                   65535, 65534, 65533, 65532, 65531, 65530,
                                   16384, 16383, 16382, 16385, 16386, 16387, 16388,
                                   -1ull, -2ull, -3ull, -4ull, -5ull, -6ull, -7ull};
    auto make_test = [&caller](const Dipole q) -> bool
    {
        return test_isqrt(U128{q.B, q.A},
                          caller);
    };
    auto get_dipole = [&choice]() -> Dipole
    {
        auto idx1 = roll_uint() % choice.size();
        auto idx2 = roll_uint() % choice.size();
        Dipole q{choice[idx1], choice[idx2]};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N)
    {
        ++counter;
        const auto dp = get_dipole();
        is_ok &= make_test(dp);
        if (!is_ok)
        {
            auto x = U128{dp.B, dp.A};
            std::cout << "x: " << x.value() << std::endl;
        }
        assert(is_ok);
        if (counter % internal_step == 0)
        {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << external_iterations << " from " << N << '\n';
        }
    }
}

void test_isqrt_randomly(long long N)
{
    if (N < 1)
    {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller<U128> caller;
    auto make_test = [&caller](const Dipole q) -> bool
    {
        return test_isqrt(U128{q.B, q.A},
                          caller);
    };
    auto get_dipole = []() -> Dipole
    {
        Dipole d{roll_ulow(), roll_ulow()};
        return d;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N)
    {
        ++counter;
        const auto dp = get_dipole();
        is_ok &= make_test(dp);
        if (!is_ok)
        {
            auto x = U128{dp.B, dp.A};
            std::cout << "x: " << x.value() << std::endl;
        }
        assert(is_ok);
        if (counter % internal_step == 0)
        {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << external_iterations << " from " << N << '\n';
        }
    }
}

void test_division_u128_semi_randomly(long long N)
{
    if (N < 1)
    {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller<U128> caller;
    const std::vector<ULOW> choice{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                   65535, 65534, 65533, 65532, 65531, 65530,
                                   16384, 16383, 16382, 16385, 16386, 16387, 16388,
                                   -1ull, -2ull, -3ull, -4ull, -5ull, -6ull, -7ull};
    auto make_test = [&caller](const Quadrupole &q, const Signess &s) -> bool
    {
        return test_div<U128>(construct_two_128bit_numbers(q, s),
                              caller);
    };
    auto get_quadrupole = [&choice]() -> Quadrupole
    {
        auto idx1 = roll_uint() % choice.size();
        auto idx2 = roll_uint() % choice.size();
        auto idx3 = roll_uint() % choice.size();
        auto idx4 = roll_uint() % choice.size();
        Quadrupole q{choice[idx1], choice[idx2], choice[idx3], choice[idx4]};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N)
    {
        ++counter;
        const Quadrupole q = get_quadrupole();
        const Signess s{roll_bool(), roll_bool()};
        if (q.is_zero_denominator())
            continue;
        is_ok &= make_test(q, s);
        assert(is_ok);
        if (counter % internal_step == 0)
        {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << external_iterations << " from " << N << '\n';
        }
    }
}

void test_division_u128_randomly(long long N)
{
    if (N < 1)
    {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller<U128> caller;
    auto make_test = [&caller](const Quadrupole &q, const Signess &s) -> bool
    {
        return test_div<U128>(construct_two_128bit_numbers(q, s),
                              caller);
    };
    auto get_quadrupole = []() -> Quadrupole
    {
        Quadrupole q{roll_ulow(), roll_ulow(), roll_ulow(), roll_ulow()};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N)
    {
        ++counter;
        const Quadrupole q = get_quadrupole();
        const Signess s{roll_bool(), roll_bool()};
        if (q.is_zero_denominator())
            continue;
        is_ok &= make_test(q, s);
        assert(is_ok);
        if (counter % internal_step == 0)
        {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << external_iterations << " from " << N << '\n';
        }
    }
}

void test_division_u256_semi_randomly(long long N)
{
    if (N < 1)
    {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller<U256> caller;
    const std::vector<ULOW> choice{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                   65535, 65534, 65533, 65532, 65531, 65530,
                                   16384, 16383, 16382, 16385, 16386, 16387, 16388,
                                   -1ull, -2ull, -3ull, -4ull, -5ull, -6ull, -7ull};
    auto make_test = [&caller](const Quadrupole &q1, const Quadrupole &q2, const Signess &s) -> bool
    {
        return test_div<U256>(construct_two_256bit_numbers(q1, q2, s),
                              caller);
    };
    auto get_quadrupole = [&choice]() -> Quadrupole
    {
        auto idx1 = roll_uint() % choice.size();
        auto idx2 = roll_uint() % choice.size();
        auto idx3 = roll_uint() % choice.size();
        auto idx4 = roll_uint() % choice.size();
        Quadrupole q{choice[idx1], choice[idx2], choice[idx3], choice[idx4]};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N)
    {
        ++counter;
        const Quadrupole q1 = get_quadrupole();
        const Quadrupole q2 = get_quadrupole();
        const Signess s{roll_bool(), roll_bool()};
        if (q1.is_zero_denominator() && q2.is_zero_denominator())
            continue;
        is_ok &= make_test(q1, q2, s);
        assert(is_ok);
        if (counter % internal_step == 0)
        {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << external_iterations << " from " << N << '\n';
        }
    }
}

void test_division_u256_randomly(long long N)
{
    if (N < 1)
    {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller<U256> caller;
    auto make_test = [&caller](const Quadrupole &q1, const Quadrupole &q2, const Signess &s) -> bool
    {
        return test_div<U256>(construct_two_256bit_numbers(q1, q2, s),
                              caller);
    };
    auto get_quadrupole = []() -> Quadrupole
    {
        Quadrupole q{roll_ulow(), roll_ulow(), roll_ulow(), roll_ulow()};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N)
    {
        ++counter;
        const Quadrupole q1 = get_quadrupole();
        const Quadrupole q2 = get_quadrupole();
        const Signess s{roll_bool(), roll_bool()};
        if (q1.is_zero_denominator() && q2.is_zero_denominator())
            continue;
        is_ok &= make_test(q1, q2, s);
        assert(is_ok);
        if (counter % internal_step == 0)
        {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << external_iterations << " from " << N << '\n';
        }
    }
}

void test_division_u512_semi_randomly(long long N)
{
    if (N < 1)
    {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller<U512> caller;
    const std::vector<ULOW> choice{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                   65535, 65534, 65533, 65532, 65531, 65530,
                                   16384, 16383, 16382, 16385, 16386, 16387, 16388,
                                   -1ull, -2ull, -3ull, -4ull, -5ull, -6ull, -7ull};
    auto make_test = [&caller](const Quadrupole &q1, const Quadrupole &q2, const Quadrupole &q3, const Quadrupole &q4, const Signess &s) -> bool
    {
        return test_div<U512>(construct_two_512bit_numbers(q1, q2, q3, q4, s),
                              caller);
    };
    auto get_quadrupole = [&choice]() -> Quadrupole
    {
        auto idx1 = roll_uint() % choice.size();
        auto idx2 = roll_uint() % choice.size();
        auto idx3 = roll_uint() % choice.size();
        auto idx4 = roll_uint() % choice.size();
        Quadrupole q{choice[idx1], choice[idx2], choice[idx3], choice[idx4]};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N)
    {
        ++counter;
        const Quadrupole q1 = get_quadrupole();
        const Quadrupole q2 = get_quadrupole();
        const Quadrupole q3 = get_quadrupole();
        const Quadrupole q4 = get_quadrupole();
        const Signess s{roll_bool(), roll_bool()};
        if (q1.is_zero_denominator() && q2.is_zero_denominator() && q3.is_zero_denominator() && q4.is_zero_denominator())
            continue;
        is_ok &= make_test(q1, q2, q3, q4, s);
        assert(is_ok);
        if (counter % internal_step == 0)
        {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << external_iterations << " from " << N << '\n';
        }
    }
}

void test_division_u512_randomly(long long N)
{
    if (N < 1)
    {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller<U512> caller;
    auto make_test = [&caller](const Quadrupole &q1, const Quadrupole &q2, const Quadrupole &q3, const Quadrupole &q4, const Signess &s) -> bool
    {
        return test_div<U512>(construct_two_512bit_numbers(q1, q2, q3, q4, s),
                              caller);
    };
    auto get_quadrupole = []() -> Quadrupole
    {
        Quadrupole q{roll_ulow(), roll_ulow(), roll_ulow(), roll_ulow()};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N)
    {
        ++counter;
        const Quadrupole q1 = get_quadrupole();
        const Quadrupole q2 = get_quadrupole();
        const Quadrupole q3 = get_quadrupole();
        const Quadrupole q4 = get_quadrupole();
        const Signess s{roll_bool(), roll_bool()};
        if (q1.is_zero_denominator() && q2.is_zero_denominator() && q3.is_zero_denominator() && q4.is_zero_denominator())
            continue;
        is_ok &= make_test(q1, q2, q3, q4, s);
        assert(is_ok);
        if (counter % internal_step == 0)
        {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << external_iterations << " from " << N << '\n';
        }
    }
}

void ferma_tests()
{
    using namespace u128::utils;
    auto check_factors = [](const std::map<U128, int> &factors, U128 x) -> bool
    {
        U128 tmp{1, 0};
        bool is_ok = true;
        for (const auto &[p, i] : factors)
        {
            is_ok &= is_prime(p);
            for (auto j = 0; j < i; ++j)
                tmp = tmp * p;
        }
        return is_ok && (x == tmp);
    };
    {
        const U128 x = U128{1129, 0} * U128{7823, 0} * U128{8, 0} * U128{81, 0} * U128{3, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{625, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{625, 0} * U128{81, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{81, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{113, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{1, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{2, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{0, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{3, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{199933, 0} * U128{999331, 0} * U128{113, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{199933, 0} * U128{999331, 0} * U128{9311, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x{8'928'986'827ull, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{2345678917ull, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{-1ull, -1ull}; // 340 282 366 920 938 463 463 374 607 431 768 211 455
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {                                                             // Медленно: ~40 sec.
        const U128 x = U128{2'153'233'344'269'007'091ull, 30ull}; // 555 555 555 555 555 555 571
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = factor(x);
        for (int c = 0; const auto &[p, i] : factors)
        {
            if (c > 0)
            {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    // { // Оч. медленно: ~9 min 12 s.
    //     std::cout << "Slow case..." << std::endl;
    //     const U128 x = U128{1'949'801'302'245'154'240ull, 47'578'344'494ull}; // 877 665 544 333 555 776 586 567 556 544
    //     std::cout << "x = " << x.value() << ", factors: {";
    //     std::cout << std::flush;
    //     auto factors = u128::factor(x);
    //     for (int c = 0; const auto& [p, i] : factors) {
    //         if (c > 0) {
    //             std::cout << ", ";
    //         }
    //         std::cout << "(" << p.value() << ", " << i << ")";
    //         c++;
    //     }
    //     std::cout << "}\n";
    //     assert(check_factors(factors, x));
    // }
}

void quadratic_residue_tests()
{
    using namespace u128::utils;
    const U128 x{15347ull, 0ull};
    {
        const U128 p{2ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(is_ok);
    }
    {
        const U128 p{17ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(is_ok);
    }
    {
        const U128 p{23ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(is_ok);
    }
    {
        const U128 p{29ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(is_ok);
    }
    {
        const U128 p{31ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(is_ok);
    }
    //
    {
        const U128 p{3ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(!is_ok);
    }
    {
        const U128 p{5ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(!is_ok);
    }
    {
        const U128 p{7ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(!is_ok);
    }
    {
        const U128 p{11ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(!is_ok);
    }
    {
        const U128 p{13ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(!is_ok);
    }
    {
        const U128 p{19ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(!is_ok);
    }
    {
        const U128 p{37ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(!is_ok);
    }
    {
        const U128 p{41ull, 0ull};
        bool is_ok = is_quadratiq_residue(x, p);
        assert(!is_ok);
    }
}

void solver_tests()
{
    using namespace solver;
    {
        Matrix<int> M{{0, 0, 0, 1}, {1, 1, 1, 0}, {1, 1, 1, 1}};
        std::vector<std::set<int>> indices = GaussJordan(M);
        bool is_ok = indices.back() == std::set<int>{0, 1, 2};
        assert(is_ok);
    }
    {
        Matrix<int> M{{0, 0, 0, 1}, {1, 1, 1, 0}};
        std::vector<std::set<int>> indices = GaussJordan(M);
        bool is_ok = indices.empty();
        assert(is_ok);
    }
}

void qs_factorization_tests()
{
    using namespace u128::utils;
    {
        U128 x{15347ull, 0};
        const auto &result = factor_qs(x, 200, 8);
        // std::cout << "QS factorization 1: {";
        // int idx = 0;
        // for (const auto& [prime, power] : result) {
        // std::cout << prime.value() << "^" << power << (idx < (result.size() - 1) ? ", " : "");
        // idx++;
        // }
        // std::cout << "}." << std::endl;
        bool is_ok = result == std::map<U128, int>{{U128{103, 0}, 1}, {U128{149, 0}, 1}};
        assert(is_ok);
    }
    const unsigned int sieve_size = 50'000u;
    for (int factor_base = 8;; factor_base++)
    {
        // std::cout << "Factor base: " << factor_base << ", sieve size: " << sieve_size << std::endl;
        const U128 x{8'928'986'827ull, 0};
        // const U128 x {140'789'674'669'022'167ull, 0};
        const auto &result = factor_qs(x, sieve_size, factor_base);
        // std::cout << "QS factorization 2: {";
        int idx = 0;
        bool factorized = false;
        for (const auto &[prime, power] : result)
        {
            // std::cout << prime.value() << "^" << power << (idx < (result.size() - 1) ? ", " : "");
            idx++;
            factorized |= (idx > 1) || (power > 1);
        }
        // std::cout << "}." << std::endl;
        if (factorized)
            break;
        // bool is_ok = result == std::map<U128, int>{{U128{74311ull, 0}, 1}, {U128{120157ull, 0}, 1}};
        // assert(is_ok);
    }
}
