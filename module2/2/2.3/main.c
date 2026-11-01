
#include "header.h"

#include <locale.h>
#include <stdio.h>
#include <windows.h>

int main(void)
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    setlocale(LC_ALL, ".UTF-8");
    setlocale(LC_NUMERIC, "C");

    Calculator calc;
    calc_init(&calc);

    if (!calc_register(&calc, '+', 1, add) ||
        !calc_register(&calc, '-', 1, subtract) ||
        !calc_register(&calc, '*', 2, multiply) ||
        !calc_register(&calc, '/', 2, divide))
    {
        fprintf(stderr, "Не удалось выделить память для операций.\n");
        calc_free(&calc);
        return 1;
    }

    run_calculator(&calc);
    calc_free(&calc);

    return 0;
}