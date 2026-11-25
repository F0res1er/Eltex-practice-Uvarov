#include "header.h"
#include <stdio.h>

static volatile sig_atomic_t stop_requested = 1;

static void handle_sigint(int signal_number)
{
    (void)signal_number;
    stop_requested = 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) 
    {
        fprintf(stderr, "Использование: %s /имя_очереди\n", argv[0]);
        return 1;
    }

    struct sigaction action = {0};
    action.sa_handler = handle_sigint;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) == -1) 
    {
        perror("sigaction");
        return 1;
    }

    Chat chat;
    if (chat_open(&chat, argv[1]) == -1) return 1;

    int result = chat_run(&chat, &stop_requested);
    chat_close(&chat);

    return result == 0 ? 0 : 1;
}