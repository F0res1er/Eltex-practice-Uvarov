#include "header.h"
#include <locale.h>
#include <stdio.h>

int main(void)
{
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");

    Calculator calc;
    calc_init(&calc);

    if (!calc_load(&calc, "./libs"))
    {
        fprintf(stderr, "Не удалось загрузить операции из каталога libs.\n");
        calc_free(&calc);
        return 1;
    }

    run_calculator(&calc);
    calc_free(&calc);

    return 0;
}
