#ifndef HEADER_H
#define HEADER_H

#include <stddef.h>

typedef double (*OperationFn)(double a, double b);

typedef struct {
    char symbol;
    int priority;
    OperationFn function;
    void* handle;
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

void calc_init(Calculator* calc);
int calc_load(Calculator* calc, const char* directory);
void calc_free(Calculator* calc);
CalcResult calc_evaluate(const Calculator* calc, const char* expression);

void run_calculator(const Calculator* calc);

#endif
