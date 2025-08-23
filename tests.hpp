#pragma once

#include <utility> // std::pair
#include <python3.10/Python.h>
#include "u128.hpp"
#include "gnumber.hpp"

using namespace u128;

using U256 = GNumber<U128, 64>;
using U512 = GNumber<U256, 128>;

/**
 * Класс для вызова методов Python и передачи результата в пространство С++.
 */
template <typename T>
class PythonCaller
{
public:
    PythonCaller();

    ~PythonCaller();

    /**
     * Делит два числа.
     * @param X Делимое.
     * @param Y Делитель.
     * @return Частное от деления, объект Python.
     */
    PyObject *Divide(T X, T Y) const;

    /**
     * Умножает два числа.
     * @param X Делимое.
     * @param Y Делитель.
     * @return Частное от деления, объект Python.
     */
    PyObject *Multiply(T X, T Y) const;

    /**
     * Извлекает целочисленный корень квадратный числа.
     * @param X Число.
     * @return Корень квадратный, объект Python.
     */
    PyObject *ISqrt(T X) const;

    /**
     * Сравнивает два частных от деления.
     * @param quotient Частное от деления Python, объект Python.
     * @param reference Частное от деления С++, строка.
     */
    bool Compare(PyObject *quotient, const char *reference) const;

private:
    PyObject *mMain = nullptr;
    PyObject *mGlobalDictionary = nullptr;
    PyObject *mLocalDictionary = nullptr;
};

/**
 * Тест деления двух чисел: сравнивается с реализацией Python.
 * @param z Делимое и делитель, соответственно.
 * @return Успех/неудача.
 */
template <typename T>
bool test_div(const std::pair<T, T> &z, PythonCaller<T> &caller);

/**
 * Тест полного умножения двух 256-битных чисел: сравнивается с реализацией Python.
 * @param z Сомножители.
 * @return Успех/неудача.
 */
bool test_256bit_mult(const std::pair<U256, U256> &z, PythonCaller<U256> &caller);

/**
 * Тест извлечения корня квадратного 128-битных чисел: сравнивается с реализацией Python.
 * @param z Число.
 * @return Успех/неудача.
 */
bool test_isqrt(U128 z, PythonCaller<U128> &caller);

/**
 * Полуслучайный тест извлечения корня квадратного 128-битных чисел, используя ограниченный набор
 * значений вблизи угловых и граничных.
 * @param N Количество внешних итераций.
 */
void test_isqrt_semi_randomly(long long N);

/**
 * Случайный тест извлечения корня квадратного 128-битных чисел.
 * @param N Количество внешних итераций.
 */
void test_isqrt_randomly(long long N);

/**
 * Полуслучайный тест деления 128-битных чисел, используя ограниченный набор
 * значений вблизи угловых и граничных.
 * @param N Количество внешних итераций.
 */
void test_division_u128_semi_randomly(long long N);

/**
 * Случайный тест деления 128-битных чисел.
 * @param N Количество внешних итераций.
 */
void test_division_u128_randomly(long long N);

/**
 * Полуслучайный тест деления 256-битных чисел, используя ограниченный набор
 * значений вблизи угловых и граничных.
 * @param N Количество внешних итераций.
 */
void test_division_u256_semi_randomly(long long N);

/**
 * Случайный тест деления 256-битных чисел.
 * @param N Количество внешних итераций.
 */
void test_division_u256_randomly(long long N);

/**
 * Полуслучайный тест деления 512-битных чисел, используя ограниченный набор
 * значений вблизи угловых и граничных.
 * @param N Количество внешних итераций.
 */
void test_division_u512_semi_randomly(long long N);

/**
 * Случайный тест деления 512-битных чисел.
 * @param N Количество внешних итераций.
 */
void test_division_u512_randomly(long long N);

/**
 * Случайный тест умножения беззнаковых 256-битных чисел с расширением до 512-бит.
 * @param N Количество внешних итераций.
 */
void test_mutliply_u256_randomly(long long N);

/**
 * Тест факторизации метода Ферма.
 */
void ferma_tests();

/**
 * Тесты функции проверки на квадратичный вычет.
 */
void quadratic_residue_tests();

/**
 * Тест факторизации метода квадратичного решета.
 */
void qs_factorization_tests();

/**
 * Тесты решателя СЛАУ.
 */
void solver_tests();