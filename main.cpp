#include <iostream>
#include "tests.hpp"
#include "gnumber.hpp"

int main(int argc, char *argv[])
{
    constexpr int g_tests = 7; // Выбор тестов для запуска.
    long long N = 3;
    if (argc > 1)
    {
        N = atoi(argv[1]);
        std::cout << "You set the number of external iterations N: " << N << '\n';
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x{U128{5}, U128{3}};
        const auto &x_str = x.value();
        std::cout << x_str << std::endl;
        assert(x_str == "1020847100762815390390123822295304634373");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x = U256::get_max_value();
        const auto &x_str = x.value();
        std::cout << x_str << std::endl;
        assert(x_str == "115792089237316195423570985008687907853269984665640564039457584007913129639935");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x{10};
        const auto &x_str = x.value();
        std::cout << x_str << std::endl;
        assert(x_str == "10");
    }
    {
        using U256 = GNumber<U128, 64>;
        using U512 = GNumber<U256, 128>;
        U512 x{12ull};
        const auto &x_str = x.value();
        std::cout << x_str << std::endl;
        assert(x_str == "12");
    }
    {
        using U256 = GNumber<U128, 64>;
        using U512 = GNumber<U256, 128>;
        U512 x{U256{12}, U256{34}};
        U512 y{U256{156}, U256{3}};
        const auto& [Q, R] = x / y;
        const auto &q_str = Q.value();
        const auto &r_str = R.value();
        std::cout << "q: " << q_str << ", r: " << r_str << std::endl;
        assert(q_str == "11");
        assert(r_str == "115792089237316195423570985008687907853269984665640564039457584007913129638232");
    }

    quadratic_residue_tests();
    solver_tests();
    qs_factorization_tests();

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
    return 0;
}