#include "header.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    char text[256];
    mode_t rights = 0;
    int known = 0, choice;

    while (1) {
        menu();
        if (!read_line(text, sizeof(text))) 
            break;

        if (strlen(text) != 1 || text[0] < '0' || text[0] > '9')
            choice = -1;
        
        else
            choice = text[0] - '0';

        switch (choice) 
        {
            case 1:
                printf("Права (например, rwxr-xr-- или 754): ");
                if (!read_line(text, sizeof(text))) return 0;

                if (parse_rights(text, &rights)) 
                {
                    known = 1;
                    show_rights(rights);
                } 
                
                else 
                {
                    puts("Неверный формат прав.");
                }
                break;

            case 2:
                printf("Имя файла: ");
                if (!read_line(text, sizeof(text))) return 0;

                if (file_rights(text, &rights)) 
                {
                    known = 1;
                    show_rights(rights);
                    printf("Для сравнения: ls -l -- \"%s\"\n", text);
                } 
                
                else 
                {
                    perror("stat");
                }
                break;

            case 3:
                if (!known) 
                {
                    puts("Сначала задайте права (пункт 1 или 2).");
                    break;
                }

                puts("Исходные права:");
                show_rights(rights);

                printf("Команда chmod (например, u+x, g-w, a=rx или 777): ");
                
                if (!read_line(text, sizeof(text))) return 0;

                if (change_rights(&rights, text)) 
                {
                    puts("Права после изменения:");
                    show_rights(rights);
                } 
                
                else 
                {
                    puts("Неверная команда.");
                }
                break;

            case 0:
                return 0;

            default:
                puts("Неверный пункт меню.");
        }
    }

    return 0;
}