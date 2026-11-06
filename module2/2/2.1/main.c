#include "header.h"
#include <locale.h>
#include <stdlib.h>
//#include <windows.h>

int main(void) {
    //SetConsoleOutputCP(CP_UTF8);
    //SetConsoleCP(CP_UTF8);

    setlocale(LC_ALL, ".UTF-8");

    PhoneBook book = { NULL, 0 };

    run_menu(&book);

    free(book.contacts);

    return 0;
}
