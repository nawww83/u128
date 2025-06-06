#include <iostream>
#include <random>
#include <cassert>
#include "tests.hpp"

static auto const seed = std::random_device{}();

static auto const internal_step = 1ll << 20;

/***
 * Генератор случайных чисел.
 */
auto roll_ulow = [urbg = std::mt19937{seed},
                distr = std::uniform_int_distribution<ULOW>{}]() mutable -> ULOW {
    return distr(urbg);
};

auto roll_uint = [urbg = std::mt19937{seed},
                distr = std::uniform_int_distribution<uint>{}]() mutable -> uint {
    return distr(urbg);
};

auto roll_bool = [urbg = std::mt19937{seed},
                distr = std::uniform_int_distribution<uint>{}]() mutable -> bool {
    return distr(urbg) % 2;
};


PythonCaller::PythonCaller()
{
    Py_Initialize();
    mMain = PyImport_AddModule("__main__");
    mGlobalDictionary = PyModule_GetDict(mMain);
    mLocalDictionary = PyDict_New();
}

PythonCaller::~PythonCaller()
{
    Py_Finalize();
}

PyObject *PythonCaller::Divide(U128 X, U128 Y) const
{
    const char* pythonScript = "quotient = nominator // denominator\n";
    PyDict_SetItemString(mLocalDictionary, "nominator", PyLong_FromString(X.value().c_str(), nullptr, 10));
    PyDict_SetItemString(mLocalDictionary, "denominator", PyLong_FromString(Y.value().c_str(), nullptr, 10));
    PyRun_String(pythonScript, Py_file_input, mGlobalDictionary, mLocalDictionary);
    return PyDict_GetItemString(mLocalDictionary, "quotient");
}

PyObject *PythonCaller::ISqrt(U128 X) const
{
    const char* pythonScript = "import math;y = math.isqrt(x)\n";
    PyDict_SetItemString(mLocalDictionary, "x", PyLong_FromString(X.value().c_str(), nullptr, 10));
    PyRun_String(pythonScript, Py_file_input, mGlobalDictionary, mLocalDictionary);
    return PyDict_GetItemString(mLocalDictionary, "y");
}

bool PythonCaller::Compare(PyObject *result, const char *reference) const
{
    PyObject* repr = PyObject_Repr(result);
    PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
    const char *bytes = PyBytes_AsString(str);
    const bool is_ok = strcmp(bytes, reference) == 0;
    if (!is_ok) {
        printf("Python: %s\n", bytes);
        printf("C++: %s\n", reference);
    }
    Py_XDECREF(repr);
    Py_XDECREF(str);
    return is_ok;
}

bool test_div(U128 z1, U128 z2, PythonCaller &caller)
{
    const auto [z3, _] = z1 / z2;
    PyObject* quotient = caller.Divide(z1, z2);
    return caller.Compare(quotient, z3.value().c_str());
}

bool test_isqrt(U128 z, PythonCaller &caller)
{
    bool exact;
    const U128 zi = isqrt(z, exact);
    const bool is_ok = exact == ((zi * zi) == z);
    PyObject* zs = caller.ISqrt(z);
    return caller.Compare(zs, zi.value().c_str()) && is_ok;
}

void test_isqrt_semi_randomly(long long N)
{
    if (N < 1) {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller caller;
    const std::vector<ULOW> choice {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
                                    65535, 65534, 65533, 65532, 65531, 65530, 
                                    16384, 16383, 16382, 16385, 16386, 16387, 16388, 
                                    -1ull, -2ull, -3ull, -4ull, -5ull, -6ull, -7ull};
    auto make_test = [&caller](const Dipole q) -> bool {
        return test_isqrt(U128{q.B, q.A},
                        caller);
    };
    auto get_dipole = [&choice]() -> Dipole {
        auto idx1 = roll_uint() % choice.size();
        auto idx2 = roll_uint() % choice.size();
        Dipole q {choice[idx1], choice[idx2]};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N) {
        ++counter;
        const auto dp = get_dipole();
        is_ok &= make_test(dp);
        if (!is_ok) {
            auto x = U128{dp.B, dp.A};
            std::cout << "x: " << x.value() << std::endl;
        }
        assert(is_ok);
        if (counter % internal_step == 0) {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << 
                external_iterations << " from " << N << '\n';
        }
    }
}

void test_isqrt_randomly(long long N)
{
    if (N < 1) {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller caller;
    auto make_test = [&caller](const Dipole q) -> bool {
        return test_isqrt(U128{q.B, q.A},
                        caller);
    };
    auto get_dipole = []() -> Dipole {
        Dipole d {roll_ulow(), roll_ulow()};
        return d;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N) {
        ++counter;
        const auto dp = get_dipole();
        is_ok &= make_test(dp);
        if (!is_ok) {
            auto x = U128{dp.B, dp.A};
            std::cout << "x: " << x.value() << std::endl;
        }
        assert(is_ok);
        if (counter % internal_step == 0) {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << 
                external_iterations << " from " << N << '\n';
        }
    }
}

void test_division_semi_randomly(long long N)
{
    if (N < 1) {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller caller;
    const std::vector<ULOW> choice {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
                                    65535, 65534, 65533, 65532, 65531, 65530, 
                                    16384, 16383, 16382, 16385, 16386, 16387, 16388, 
                                    -1ull, -2ull, -3ull, -4ull, -5ull, -6ull, -7ull};
    auto make_test = [&caller](const Quadrupole q, const Signess s) -> bool {
        return test_div(U128{q.B, q.A, u128::Sign{s.s1}},
                        U128{q.D, q.C, u128::Sign{s.s2}},
                        caller);
    };
    auto get_quadrupole = [&choice]() -> Quadrupole {
        auto idx1 = roll_uint() % choice.size();
        auto idx2 = roll_uint() % choice.size();
        auto idx3 = roll_uint() % choice.size();
        auto idx4 = roll_uint() % choice.size();
        Quadrupole q {choice[idx1], choice[idx2], choice[idx3], choice[idx4]};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N) {
        ++counter;
        const Quadrupole q = get_quadrupole();
        const Signess s{roll_bool(), roll_bool()};
        if (q.C == 0 && q.D == 0) {
            continue;
        }
        is_ok &= make_test(q, s);
        assert(is_ok);
        if (counter % internal_step == 0) {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << 
                external_iterations << " from " << N << '\n';
        }
    }
}

void test_division_randomly(long long N)
{
    if (N < 1) {
        std::cout << "Skipped!\n";
        return;
    }
    PythonCaller caller;
    auto make_test = [&caller](const Quadrupole q, const Signess s) -> bool {
        return test_div(U128{q.B, q.A, u128::Sign{s.s1}},
                        U128{q.D, q.C, u128::Sign{s.s2}},
                        caller);
    };
    auto get_quadrupole = []() -> Quadrupole {
        Quadrupole q {roll_ulow(), roll_ulow(), roll_ulow(), roll_ulow()};
        return q;
    };
    long long counter = 0;
    long long external_iterations = 0;
    bool is_ok = true;
    while (external_iterations < N) {
        ++counter;
        const Quadrupole q = get_quadrupole();
        const Signess s{roll_bool(), roll_bool()};
        if (q.C == 0 && q.D == 0) {
            continue;
        }
        is_ok &= make_test(q, s);
        assert(is_ok);
        if (counter % internal_step == 0) {
            external_iterations++;
            std::cout << "... iterations: " << counter << ". External: " << 
                external_iterations << " from " << N << '\n';
        }
    }
}

void ferma_tests() {
    auto check_factors = [](const std::map<U128, int>& factors, U128 x) -> bool {
        U128 tmp {1, 0};
        bool is_ok = true;
        for (const auto& [p, i] : factors) {
            is_ok &= u128::is_prime(p);
            for (auto j = 0; j < i; ++j)
                tmp = tmp * p;
        }
        return is_ok && (x == tmp);
    };
    {
        const U128 x = U128{1129, 0} * U128{7823, 0} * U128{8, 0} * U128{81, 0} * U128{3, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    {
        const U128 x = U128{199933, 0} * U128{999331, 0}* U128{113, 0};
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
                std::cout << ", ";
            }
            std::cout << "(" << p.value() << ", " << i << ")";
            c++;
        }
        std::cout << "}\n";
        assert(check_factors(factors, x));
    }
    { // Медленно: ~40 sec.
        const U128 x = U128{2'153'233'344'269'007'091ull, 30ull}; // 555 555 555 555 555 555 571
        std::cout << "x = " << x.value() << ", factors: {";
        std::cout << std::flush;
        auto factors = u128::factor(x);
        for (int c = 0; const auto& [p, i] : factors) {
            if (c > 0) {
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