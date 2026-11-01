#ifndef HEADER_H
#define HEADER_H

#include <stddef.h>

// Указатель на математическую операцию.
typedef double (*OperationFn)(double a, double b);

typedef struct {
    char symbol;
    int priority;
    OperationFn function;
} Operation;

typedef struct {
    Operation* operations;
    size_t count;
} Calculator;

typedef enum {
    CALC_OK,
    CALC_SYNTAX_ERROR
} CalcError;

typedef struct {
    double value;
    CalcError error;
    size_t error_position;
    int division_by_zero;
} CalcResult;

// Регистрация операций и вычисление выражения
void calc_init(Calculator* calc);
int calc_register(Calculator* calc, char symbol, int priority, OperationFn function);
void calc_free(Calculator* calc);
CalcResult calc_evaluate(const Calculator* calc, const char* expression);

double add(double a, double b);
double subtract(double a, double b);
double multiply(double a, double b);
double divide(double a, double b);

// ИнтерфЭйс чепухи
void run_calculator(const Calculator* calc);

#endif