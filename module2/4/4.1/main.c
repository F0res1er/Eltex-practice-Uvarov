#include "header.h"
#include <locale.h>

int main(void)
{
    setlocale(LC_ALL, ".UTF-8");

    PhoneBook book = { NULL, 0 };

    run_menu(&book);

    while (book.head != NULL)
        delete_contact(&book, book.head);

    return 0;
}