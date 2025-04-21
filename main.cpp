#include <iostream>
#include "tests.hpp"

int main(int argc, char* argv[]) {
    constexpr int g_tests = 4; // Выбор тестов для запуска.
    long long N = 3;
    if (argc > 1) {
        N = atoi(argv[1]);
        std::cout << "You set the number of external iterations N: " << N << '\n';
    }
    { // An example of division of two 128-bit numbers.
        using namespace u128;
        U128 z1 {345, 47}; // (47*2^64 + 345) / (3*2^64 + 6632)
        U128 z2 {6632, 3};
        auto [result, remainder] = z1 / z2;
        std::cout << "Example: " << z1.value() << " / " << z2.value() << " = " << result.value() << 
                ", remainder: " << remainder.value() << '\n';
    }

    if (g_tests & 0x1) {
        std::cout << "Run semi-random division test...\n";
        test_division_semi_randomly(N);
        std::cout << "Ok\n";

        std::cout << "Run random division test...\n";
        test_division_randomly(N);
        std::cout << "Ok\n";
    }

    if (g_tests & 0b10) {
        std::cout << "Run semi-random isqrt test...\n";
        test_isqrt_semi_randomly(N);
        std::cout << "Ok\n";

        std::cout << "Run random isqrt test...\n";
        test_isqrt_randomly(N);
        std::cout << "Ok\n";
    }
    if (g_tests & 0b100) {
        std::cout << "Run Ferma factorization method test...\n";
        ferma_tests();
        std::cout << "Ok\n";
    }
    return 0;
}