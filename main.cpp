#include <iostream>
#include "tests.hpp"
#include "gnumber.hpp"

int main(int argc, char *argv[])
{
    constexpr int g_tests = 8; // Выбор тестов для запуска.
    long long N = 3;
    if (argc > 1)
    {
        N = atoi(argv[1]);
        std::cout << "You set the number of external iterations N: " << N << '\n';
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x {U128{5}, U128{3}};
        const auto& x_str = x.value();
        std::cout << x_str << std::endl;
        assert(x_str == "1020847100762815390390123822295304634373");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x = U256::get_max_value();
        const auto& x_str = x.value();
        std::cout << x_str << std::endl;
        assert(x_str == "115792089237316195423570985008687907853269984665640564039457584007913129639935");
    }
    // {
    //     using U256 = GNumber<U128, 64>;
    //     // Q: 3402823669209384634633746074317
    //     // N: 232111556988034494792395846463493303575
    //     U256 x;
    //     x.mLow = U128{6366505597545540887ull, 12582792717270998368ull};
    //     x.mHigh = U128{1761962158423493325ull, 184467440737ull};
    //     // x = x * U128{10u};
    //     std::cout << x.mLow.value() << std::endl;
    //     U256 v = U256::mult128(x.mLow, U128{10});
    //     // U256 w = U256::mult128(x.mHigh, U128{10});
    //     std::cout << v.mHigh.value() << " : " << v.mLow.value() << std::endl;
    //     // std::cout << w.mHigh.value() << " : " << w.mLow.value() << std::endl;
    // }
    // {
    //     using U256 = GNumber<U128, 64>;
    //     U256 x {U128{8324823754326754064ull, 15147462730452673987ull}, U128{17619621584234933256ull, 1844674407370ull}};
    //     x = x.div10();
    //     std::cout << x.mHigh.value() << " : " << x.mLow.value() << std::endl;
    // }
    // {
    //     using U256 = GNumber<U128, 64>;
    //     U256 x = U256::get_max_value();
    //     for (int i = 0; i < 12; ++i) {
    //         std::cout << "HL: " << x.mHigh.value() << " : " << x.mLow.value() << std::endl;
    //         x = x.div10();
    //     }
    // }

    // qs_factorization_tests();
    // solver_tests();

    if (g_tests & 0x1)
    {
        std::cout << "Run semi-random division test...\n";
        test_division_semi_randomly(N);
        std::cout << "Ok\n";

        std::cout << "Run random division test...\n";
        test_division_randomly(N);
        std::cout << "Ok\n";
    }

    if (g_tests & 0b10)
    {
        std::cout << "Run semi-random isqrt test...\n";
        test_isqrt_semi_randomly(N);
        std::cout << "Ok\n";

        std::cout << "Run random isqrt test...\n";
        test_isqrt_randomly(N);
        std::cout << "Ok\n";
    }
    if (g_tests & 0b100)
    {
        std::cout << "Run Ferma factorization method test...\n";
        ferma_tests();
        std::cout << "Ok\n";
    }
    // quadratic_residue_tests();
    return 0;
}