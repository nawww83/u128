#pragma once

#include <utility> // std::pair
#include <python3.10/Python.h>
#include "u128.hpp"

using namespace u128;

/**
 * Класс для вызова методов Python и передачи результата в пространство С++.
 */
template <typename T>
class PythonCaller {
public:
    PythonCaller();

    ~PythonCaller();

    /**
     * Делит два 128-битных числа.
     * @param X Делимое.
     * @param Y Делитель.
     * @return Частное от деления, объект Python.
    */
    PyObject* Divide(T X, T Y) const;

    /**
     * Извлекает целочисленный корень квадратный 128-битного числа.
     * @param X Число.
     * @return Корень квадратный, объект Python.
    */
   PyObject* ISqrt(T X) const;

    /**
     * Сравнивает два частных от деления.
     * @param quotient Частное от деления Python, объект Python.
     * @param reference Частное от деления С++, строка.
    */
    bool Compare(PyObject* quotient, const char* reference) const;
private:

    PyObject* mMain = nullptr;
    PyObject* mGlobalDictionary = nullptr;
    PyObject* mLocalDictionary = nullptr;
};

/**
 * Тест деления двух чисел: сравнивается с реализацией Python.
 * @param z Делимое и делитель, соответственно.
 * @return Успех/неудача.
*/
template <typename T>
bool test_div(const std::pair<T, T>& z, PythonCaller<T>& caller);

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