#include "header.h"
#include <stdio.h>
#include <stdlib.h>

void run(Queue *queue)
{
    int option, value, count, next_id = 1;
    Message result;

    while (1) {
        printf("\n1 - Добавить сообщение\n"
               "2 - Сгенерировать сообщения\n"
               "3 - Извлечь первое\n"
               "4 - Извлечь с указанным приоритетом\n"
               "5 - Извлечь с приоритетом не ниже заданного\n"
               "6 - Показать очередь\n"
               "0 - Выход\nВыбор: ");

        if (scanf("%d", &option) != 1)
            break;

        switch (option) {
            case 0:
                return;

            case 1:
                printf("Приоритет (0-255): ");
                if (scanf("%d", &value) != 1)
                    return;

                if (value < 0 || value > 255) 
                {
                    puts("Приоритет должен быть от 0 до 255.");
                    break;
                }

                if (!enqueue(queue, next_id, (unsigned char)value)) 
                {
                    puts("Ошибка выделения памяти.");
                    return;
                }

                printf("Добавлено сообщение %d, приоритет %d\n", next_id, value);
                next_id++;
                break;

            case 2:
                printf("Количество сообщений: ");
                
                if (scanf("%d", &count) != 1)
                    return;

                for (int i = 0; i < count; i++) 
                {
                    int priority = rand() % 256;

                    if (!enqueue(queue, next_id, (unsigned char)priority)) 
                    {
                        puts("Ошибка выделения памяти.");
                        return;
                    }

                    printf("Добавлено сообщение %d, приоритет %d\n", next_id, priority);
                    next_id++;
                }
                break;

            case 3:
            case 4:
            case 5:
                value = 0;

                if (option != 3) 
                {
                    printf("Приоритет (0-255): ");
                    
                    if (scanf("%d", &value) != 1)
                        return;

                    if (value < 0 || value > 255) 
                    {
                        puts("Приоритет должен быть от 0 до 255.");
                        break;
                    }
                }

                if (dequeue(queue, (ExtractMode)(option - 2), (unsigned char)value, &result)) 
                {
                    printf("Извлечено сообщение %d, приоритет %u\n", result.id, (unsigned int)result.priority);
                } 
                
                else 
                {
                    puts("Подходящих сообщений нет.");
                }

                break;

            case 6:
                printQueue(queue);
                break;

            default:
                puts("Неизвестная команда.");
                break;
        }
    }
}