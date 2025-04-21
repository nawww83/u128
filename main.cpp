#include <iostream>
#include "tests.hpp"

int main(int argc, char* argv[]) {
    long long N = 10;
    if (argc > 1) {
        N = atoi(argv[1]);
        std::cout << "You set the number of external iterations N: " << N << '\n';
    }
    // { // An example of division of two 128-bit numbers.
    //     using namespace u128;
    //     U128 z1 {345, 47}; // (47*2^64 + 345) / (3*2^64 + 6632)
    //     U128 z2 {6632, 3};
    //     U128 result = z1 / z2;
    //     std::cout << "Example: " << z1.value() << " / " << z2.value() << " = " << result.value() << '\n';
    // }
    // std::cout << "Run semi-random division test...\n";
    // test_division_semi_randomly(N);
    // std::cout << "Ok\n";

    // std::cout << "Run random division test...\n";
    // test_division_randomly(N);
    // std::cout << "Ok\n";

    // std::cout << "Run semi-random isqrt test...\n";
    // test_isqrt_semi_randomly(N);
    // std::cout << "Ok\n";

    // std::cout << "Run random isqrt test...\n";
    // test_isqrt_randomly(N);
    // std::cout << "Ok\n";

    auto check_factors = [](const std::map<U128, int>& factors, U128 x) -> bool {
        U128 tmp {1, 0};
        for (const auto& [p, i] : factors) {
            for (auto j = 0; j < i; ++j)
                tmp = tmp * p;
        }
        return x == tmp;
    };
    {
        const U128 x = U128{1129, 0} * U128{7823, 0} * U128{8, 0} * U128{81, 0} * U128{3, 0};
        auto factors = u128::factor(x);
        std::cout << "x = " << x.value() << ", factors: {";
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
        auto factors = u128::factor(x);
        std::cout << "x = " << x.value() << ", factors: {";
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
        auto factors = u128::factor(x);
        std::cout << "x = " << x.value() << ", factors: {";
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
        auto factors = u128::factor(x);
        std::cout << "x = " << x.value() << ", factors: {";
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
        auto factors = u128::factor(x);
        std::cout << "x = " << x.value() << ", factors: {";
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
        // const U128 x = U128{199933, 0} * U128{999331, 0} * U128{9311, 0}; // Медленно.
        const U128 x = U128{199933, 0} * U128{999331, 0}* U128{113, 0};
        auto factors = u128::factor(x);
        std::cout << "x = " << x.value() << ", factors: {";
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
    return 0;
}