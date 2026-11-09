#include "header.h"
#include <locale.h>

int main(void)
{
    setlocale(LC_ALL, "");

    PhoneBook book = {0};

    run_menu(&book);

    free_contacts(book.root);

    return 0;
}
