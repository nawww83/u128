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

    std::cout << "Run semi-random isqrt test...\n";
    test_isqrt_semi_randomly(N);
    std::cout << "Ok\n";

    std::cout << "Run random isqrt test...\n";
    test_isqrt_randomly(N);
    std::cout << "Ok\n";

    return 0;
}