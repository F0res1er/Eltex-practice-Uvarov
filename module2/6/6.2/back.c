#include "header.h"
#include <stdlib.h>
#include <string.h>

static int compare_contacts(const Contact* a, const Contact* b)
{
    int result = strcmp(a->surname, b->surname);

    if (result != 0)
        return result;

    result = strcmp(a->name, b->name);

    if (result != 0)
        return result;

    return strcmp(a->patronymic, b->patronymic);
}

// Вставка узла в упорядоченный список
static void insert_sorted(PhoneBook* book, ContactNode* node)
{
    ContactNode* current = book->head;
    ContactNode* previous = NULL;

    while (current != NULL &&
        compare_contacts(&current->contact, &node->contact) <= 0)
    {
        previous = current;
        current = current->next;
    }

    node->prev = previous;
    node->next = current;

    if (previous != NULL)
        previous->next = node;
    else
        book->head = node;

    if (current != NULL)
        current->prev = node;
}

// Добавление контакта
int add_contact(PhoneBook* book, const Contact* contact)
{
    ContactNode* node = malloc(sizeof(ContactNode));

    if (node == NULL)
        return 0;

    node->contact = *contact;

    insert_sorted(book, node);
    book->count++;

    return 1;
}

// Редактирование контакта
void edit_contact(PhoneBook* book, ContactNode* node, const Contact* contact)
{
    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        book->head = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    node->contact = *contact;

    insert_sorted(book, node);
}

// Удаление контакта
void delete_contact(PhoneBook* book, ContactNode* node)
{
    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        book->head = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    free(node);
    book->count--;
}