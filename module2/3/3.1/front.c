#include "header.h"
#include <stdio.h>
#include <string.h>

void menu(void) 
{
    printf("\n1. Ввести права\n"
           "2. Права файла\n"
           "3. Изменить права\n"
           "0. Выход\nВыбор: ");
}

int read_line(char *s, size_t size) 
{
    if (!fgets(s, size, stdin)) return 0;
    
    s[strcspn(s, "\n")] = '\0';
    return 1;
}

void show_rights(mode_t mode)
{
    char s[] = "---------";

    for (int i = 0; i < 9; i++)
        if (mode & (0400 >> i)) //побитовым И проверяем установлен ли соответствующий бит на позиции
            s[i] = "rwx"[i % 3]; //Если да - накидываем буковку

    printf("Буквенное: %s\n"
           "Цифровое: %03o\n"
           "Битовое: ", s, (unsigned)(mode & 0777));

    for (int i = 8; i >= 0; i--)
        printf("%d", (int)((mode >> i) & 1));

    putchar('\n');
}