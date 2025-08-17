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
        U256 x{U128{5}, U128{3}};
        const auto &x_str = x.value();
        assert(x_str == "1020847100762815390390123822295304634373");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x = U256::get_max_value();
        const auto &x_str = x.value();
        assert(x_str == "115792089237316195423570985008687907853269984665640564039457584007913129639935");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x{10};
        const auto &x_str = x.value();
        assert(x_str == "10");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x1{U128{0}, U128{0, 65532}};
        U128 x2{-1ull, -1ull};
        const auto &x1_str = x1.value();
        assert(x1_str == "411351030923359963815686952481644795580019556986617109699100672");
        auto [q, r] = x1 / x2;
        assert(q.value() == "1208852032638334336499712");
        assert(r.value() == "1208852032638334336499712");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x1{U128{65533, 65533}, U128{0, 65532}};
        U256 x2{U128{18446744073709551614ull, 18446744073709551615ull}, U128{65534, 18446744073709551612ull}};
        auto [q, r] = x1 / x2;
        const auto &q_str = q.value();
        const auto &r_str = r.value();
        assert(q_str == "0");
        assert(r_str == "411351030923359963815686952481644795581228427465999517745217533");
    }
    {
        using U256 = GNumber<U128, 64>;
        using U512 = GNumber<U256, 128>;
        U512 x{12ull};
        const auto &x_str = x.value();
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
        assert(q_str == "11");
        assert(r_str == "115792089237316195423570985008687907853269984665640564039457584007913129638232");
    }

    quadratic_residue_tests();
    solver_tests();
    qs_factorization_tests();

    std::cout << "Run semi-random U256 division test...\n";
    test_division_u256_semi_randomly(N);
    std::cout << "Ok\n";

    std::cout << "Run random U256 division test...\n";
    test_division_u256_randomly(N);
    std::cout << "Ok\n";

    if (g_tests & 0x1)
    {
        std::cout << "Run semi-random U128 division test...\n";
        test_division_u128_semi_randomly(N);
        std::cout << "Ok\n";

        std::cout << "Run random U128 division test...\n";
        test_division_u128_randomly(N);
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