#include "header.h"
#include <stdlib.h>

// Добавление контакта
int add_contact(PhoneBook* book, const Contact* contact) 
{
    Contact* temp = realloc(book->contacts, (book->count + 1) * sizeof(Contact));

    if (temp == NULL) 
    {
        return 0;
    }

    book->contacts = temp;
    book->contacts[book->count] = *contact;
    book->count++;

    return 1;
}

void edit_contact(PhoneBook* book, size_t index, const Contact* contact) 
{
    book->contacts[index] = *contact;
}

void delete_contact(PhoneBook* book, size_t index) 
{
    for (size_t i = index; i + 1 < book->count; i++) 
    {
        book->contacts[i] = book->contacts[i + 1];
    }

    book->count--;
}
