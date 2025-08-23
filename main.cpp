#include <iostream>
#include <cstdint> // uint64_t
#include "tests.hpp"
#include "gnumber.hpp"

int main(int argc, char *argv[])
{
    uint64_t g_tests = 16ull; // Выбор тестов для запуска.
    long long N = 3;
    if (argc > 1)
    {
        N = atoi(argv[1]);
        std::cout << "You set the number of external iterations N: " << N << '\n';
    }
    if (argc > 2)
    {
        g_tests = std::stoull(argv[2]);
        std::cout << "You set the test selector: " << g_tests << '\n';
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
        U256 x1{U128{16385, 18446744073709551610ull}, U128{18446744073709551613ull, 18446744073709551614ull}};
        U256 x2{U128{18446744073709551613ull, 18446744073709551614ull}, U128{18446744073709551613ull, 0}};
        auto [q, r] = x1 / x2;
        const auto &q_str = q.value();
        const auto &r_str = r.value();
        assert(q_str == "18446744073709551617");
        assert(r_str == "340282366920938463426481119284349124612");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x1{U128{1, 0}, U128{0, 0}};
        U256 x2{U128{0, 0}, U128{0, 0}};
        auto sub = U256::sub_mod(x1, x2);
        const auto &sub_str = sub.value();
        assert(sub_str == "1");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x1{U128{0, 0}, U128{0, 0}};
        U256 x2{U128{1, 0}, U128{0, 0}};
        auto sub = U256::sub_mod(x1, x2);
        const auto &sub_str = sub.value();
        assert(sub_str == "115792089237316195423570985008687907853269984665640564039457584007913129639935");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x1{U128{0, 0}, U128{0, 0}};
        U256 x2{U128{0, 0}, U128{1, 0}};
        auto sub = U256::sub_mod(x1, x2);
        const auto &sub_str = sub.value();
        assert(sub_str == "115792089237316195423570985008687907852929702298719625575994209400481361428480");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x1{U128{1, 0}, U128{0, 0}};
        U256 x2{U128{0, 0}, U128{1, 0}};
        auto sub = U256::sub_mod(x1, x2);
        const auto &sub_str = sub.value();
        assert(sub_str == "115792089237316195423570985008687907852929702298719625575994209400481361428481");
    }
    {
        using U256 = GNumber<U128, 64>;
        U256 x1{U128{0, 0}, U128{0, 0}};
        U256 x2{U128{1, 0}, U128{1, 0}};
        auto sub = U256::sub_mod(x1, x2);
        const auto &sub_str = sub.value();
        assert(sub_str == "115792089237316195423570985008687907852929702298719625575994209400481361428479");
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
        U512 x{U256{2}, U256{2}};
        U512 y{U256{1}, U256{1}};
        const auto &[Q, R] = x / y;
        const auto &q_str = Q.value();
        const auto &r_str = R.value();
        assert(q_str == "2");
        assert(r_str == "0");
    }

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
    if (g_tests & 0b1000)
    {
        std::cout << "Run semi-random U256 division test...\n";
        test_division_u256_semi_randomly(N);
        std::cout << "Ok\n";

        std::cout << "Run random U256 division test...\n";
        test_division_u256_randomly(N);
        std::cout << "Ok\n";
    }
    if (g_tests & 0b10000)
    {
        std::cout << "Run semi-random U512 division test...\n";
        test_division_u512_semi_randomly(N);
        std::cout << "Ok\n";

        std::cout << "Run random U512 division test...\n";
        test_division_u512_randomly(N);
        std::cout << "Ok\n";
    }
    if (g_tests && 0b100000)
    {
        std::cout << "Quadratic residue test...\n";
        quadratic_residue_tests();
        std::cout << "Ok\n";

        std::cout << "Solver test...\n";
        solver_tests();
        std::cout << "Ok\n";

        std::cout << "QS factorization test...\n";
        qs_factorization_tests();
        std::cout << "Ok\n";
    }
    if (g_tests & 0b1000000)
    {
        std::cout << "Run random U256 full multiplication test...\n";
        test_mutliply_u256_randomly(N);
        std::cout << "Ok\n";
    }

    std::cout << "All is ok!" << std::endl;
    return 0;
}