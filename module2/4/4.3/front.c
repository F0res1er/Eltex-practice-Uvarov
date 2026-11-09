#include "header.h"
#include <stdio.h>
#include <string.h>

static void read_text(const char *message, char *field, size_t size)
{
    printf("%s", message);

    if (fgets(field, size, stdin) != NULL)
        field[strcspn(field, "\n")] = '\0';
}

static int read_number(const char *message)
{
    char line[32];
    int number;

    printf("%s", message);

    if (fgets(line, sizeof(line), stdin) == NULL || sscanf(line, "%d", &number) != 1)
        return -1;

    return number;
}

static int valid_name(const char *text, int required)
{
    if (text[0] == '\0')
        return !required;

    for (size_t i = 0; text[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)text[i];

        if (c < 128 && !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) return 0;
    }

    return 1;
}

static int valid_phone(const char *text)
{
    for (size_t i = 0; text[i] != '\0'; i++) 
    {
        if (text[i] < '0' || text[i] > '9')
            return 0;
    }

    return 1;
}

static void read_name(const char *message, char *field, int required)
{
    do {
        read_text(message, field, MAX_STR);

        if (valid_name(field, required))
            return;

        printf("Ошибка: допускаются только буквы");

        if (required)
            printf(", поле обязательно");

        printf(".\n");

    } while (1);
}

static void read_phone(const char *message, char *field)
{
    do {
        read_text(message, field, MAX_STR);

        if (valid_phone(field))
            return;

        printf("Ошибка: номер должен содержать только цифры.\n");

    } while (1);
}

static void input_contact(Contact *contact)
{
    printf("* - обязательный ввод\n");

    read_name("* Фамилия: ", contact->surname, 1);
    read_name("* Имя: ", contact->name, 1);
    read_name("Отчество: ", contact->patronymic, 0);

    read_text("Место работы: ", contact->workplace, MAX_STR);
    read_text("Должность: ", contact->position, MAX_STR);

    for (int i = 0; i < MAX_PHONES; i++) {
        char message[40];

        snprintf(message, sizeof(message), "Телефон %d: ", i + 1);
        read_phone(message, contact->phones[i]);
    }

    for (int i = 0; i < MAX_EMAILS; i++) {
        char message[40];

        snprintf(message, sizeof(message), "Email %d: ", i + 1);
        read_text(message, contact->emails[i], MAX_STR);
    }

    read_text("Telegram: ", contact->social.telegram, MAX_STR);
    read_text("VK: ", contact->social.vk, MAX_STR);
    read_text("Другие соц-сети: ", contact->social.other, MAX_STR);
}

static void show_nodes(const ContactNode *node, size_t *number)
{
    if (node == NULL)
        return;

    show_nodes(node->left, number);

    printf("%zu. %s %s\n", ++(*number), node->contact.surname, node->contact.name);

    show_nodes(node->right, number);
}

static void show_list(const PhoneBook *book)
{
    if (book->count == 0) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    size_t number = 0;

    show_nodes(book->root, &number);
}

static void show_contact(const Contact *contact)
{
    printf("Ф.И.О.: %s %s %s\n", contact->surname, contact->name, contact->patronymic);

    if (contact->workplace[0])
        printf("Место работы: %s\n", contact->workplace);

    if (contact->position[0])
        printf("Должность: %s\n", contact->position);

    for (int i = 0; i < MAX_PHONES; i++) 
    {
        if (contact->phones[i][0])
            printf("Телефон %d: %s\n", i + 1, contact->phones[i]);
    }

    for (int i = 0; i < MAX_EMAILS; i++) 
    {
        if (contact->emails[i][0])
            printf("Email %d: %s\n", i + 1, contact->emails[i]);
    }

    if (contact->social.telegram[0])
        printf("Telegram: %s\n", contact->social.telegram);

    if (contact->social.vk[0])
        printf("VK: %s\n", contact->social.vk);

    if (contact->social.other[0])
        printf("Другие соц-сети: %s\n", contact->social.other);
}

void run_menu(PhoneBook *book)
{
    int choice;

    do {
        printf("\n====ТЕЛЕФОННАЯ КНИГА====\n");
        printf("1. Добавить контакт\n");
        printf("2. Показать список\n");
        printf("3. Просмотреть контакт\n");
        printf("4. Редактировать контакт\n");
        printf("5. Удалить контакт\n");
        printf("0. Выход\n");

        choice = read_number("Выбор: ");

        switch (choice) {
        case 1: {
            Contact contact = {0};

            input_contact(&contact);

            if (add_contact(book, &contact))
                printf("Контакт добавлен.\n");
            
            else
                printf("Ошибка выделения памяти.\n");

            break;
        }

        case 2:
            show_list(book);
            break;

        case 3:
        case 4:
        case 5: {
            show_list(book);

            if (book->count == 0)
                break;

            int id = read_number("Номер контакта: ");

            if (id < 1 || (size_t)id > book->count) 
            {
                printf("Неверный номер контакта.\n");
                break;
            }

            if (choice == 3) 
            {
                show_contact(get_contact(book, (size_t)id - 1));
            }

            else if (choice == 4) 
            {
                Contact contact = {0};

                printf("Введите новые данные контакта:\n");

                input_contact(&contact);

                if (edit_contact(book, (size_t)id - 1, &contact))
                    printf("Контакт изменён.\n");
                
                else
                    printf("Ошибка выделения памяти.\n");
            }

            else 
            {
                delete_contact(book, (size_t)id - 1);
                printf("Контакт удалён.\n");
            }

            break;
        }

        case 0:
            break;

        default:
            printf("Неверная команда.\n");
        }

    } while (choice != 0);
}