#pragma once

#include <python3.10/Python.h>
#include "u128.hpp"

using namespace u128;

/**
 * Класс для вызова методов Python и передачи результата в пространство С++.
 */
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
    PyObject* Divide(U128 X, U128 Y) const;

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
 * Тест деления двух конкретных 128-битных чисел: сравнивается с реализацией Python.
 * @param z1 Делимое.
 * @param z2 Делитель.
 * @return Успех/неудача.
*/
bool test_div(U128 z1, U128 z2, PythonCaller& caller);

/**
 * Полуслучайный тест деления 128-битных чисел, используя ограниченный набор 
 * значений вблизи угловых и граничных.
 * @param N Количество внешних итераций.
*/
void test_division_semi_randomly(long long N);


/**
 * Случайный тест деления 128-битных чисел.
 * @param N Количество внешних итераций.
*/
void test_division_randomly(long long N);