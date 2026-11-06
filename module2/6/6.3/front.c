#include "header.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define INPUT_SIZE 1000

static char* trim(char* s)
{
    while (isspace((unsigned char)*s)) ++s;

    size_t len = strlen(s);

    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';

    return s;
}

void run_calculator(const Calculator* calc)
{
    char buffer[INPUT_SIZE];

    puts("Калькулятор: введите математическое выражение.");
    puts("Для выхода введите exit.");

    for (;;)
    {
        fputs("> ", stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            putchar('\n');
            break;
        }

        if (strchr(buffer, '\n') == NULL && !feof(stdin))
        {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF);

            puts("Ошибка: выражение слишком длинное.");
            continue;
        }

        char* line = trim(buffer);

        if (*line == '\0') continue;

        if (strcmp(line, "exit") == 0) break;

        CalcResult result = calc_evaluate(calc, line);

        if (result.error != CALC_OK)
        {
            printf("Ошибка в позиции %zu.\n", result.error_position + 1);
            continue;
        }

        if (isnan(result.value))
        {
            puts("Результат: NaN (не число)");
        }

        else if (isinf(result.value))
        {
            printf("Результат: %sinf (бесконечность)\n", signbit(result.value) ? "-" : "+");
        }

        else
        {
            printf("Результат: %.15g\n", result.value);
        }

        if (result.division_by_zero)
        {
            puts("Предупреждение: произошло деление на ноль.");
        }
    }
    puts("Завершение работы.");
}