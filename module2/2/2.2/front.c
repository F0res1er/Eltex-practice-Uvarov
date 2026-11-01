#include "header.h"
#include <stdio.h>
#include <math.h>

// Очищение текущей строки ввода
static void clear_input() 
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Меню
void show_menu() 
{
    printf("\n=== Калькулятор ===\n");
    printf("1. Сложение (+)\n");
    printf("2. Вычитание (-)\n");
    printf("3. Умножение (*)\n");
    printf("4. Деление (/)\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}

// Получение пункта меню
int get_choice()
{
    int choice;
    int c;
    int status;

    status = scanf("%d", &choice);

    if (status == EOF) return 0;

    if (status != 1)
    {
        printf("Ошибка ввода. Попробуйте снова.\n");
        clear_input();
        return -1;
    }

    // Проверка символов после числа
    while ((c = getchar()) == ' ' || c == '\t');

    if (c != '\n' && c != EOF)
    {
        printf("Ошибка: присутствуют букавы и прочая ересь!\n");
        clear_input();
        return -1;
    }

    if (choice < 0 || choice > 4)
    {
        printf("Неверный выбор. Попробуйте снова.\n");
        return -1;
    }

    return choice;
}

// Получение числа
int get_number(const char* message, double* number)
{
    int c;
    printf("%s", message);

    if (scanf("%lf", number) != 1)
    {
        printf("Ошибка ввода числа.\n");
        clear_input();
        return 0;
    }
    // Проверка символов после числа
    while ((c = getchar()) == ' ' || c == '\t');

    if (c != '\n' && c != EOF)
    {
        printf("Ошибка: присутствуют букавы и прочая ересь!\n");
        clear_input();
        return 0;
    }

    return 1;
}

// Выбор операции и вывод результата
void show_operation(int choice) 
{
    double a, b, result;
    char symbol;

    if (!get_number("Введите первое число: ", &a)) return;

    if (!get_number("Введите второе число: ", &b)) return;

    switch (choice) 
    {
    case 1:
        result = add(a, b);
        symbol = '+';
        break;
    case 2:
        result = subtract(a, b);
        symbol = '-';
        break;
    case 3:
        result = multiply(a, b);
        symbol = '*';
        break;
    case 4:
        result = divide(a, b);
        symbol = '/';
        break;
    default:
        return;
    }

    if (choice == 4 && isnan(result)) 
    {
        printf("Ошибка: деление на ноль!\n");
        return;
    }

    printf("%.2f %c %.2f = %.2f\n", a, symbol, b, result);
}

// Основной цикл
void run_calculator(void) 
{
    int choice;

    while (1) 
    {
        show_menu();
        choice = get_choice();

        if (choice == 0)
        {
            printf("Выход из программы.\n");
            return;
        }
        
        if (choice == -1) continue;

        show_operation(choice);
    }
}