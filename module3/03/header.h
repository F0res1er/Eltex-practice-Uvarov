#ifndef HEADER_H
#define HEADER_H

#include <mqueue.h>
#include <signal.h>

#define MESSAGE_SIZE 256
#define QUEUE_NAME_SIZE 256
#define MESSAGE_PRIORITY 1
#define EXIT_PRIORITY 10

typedef struct {
    mqd_t queue1;
    mqd_t queue2;
    char name1[QUEUE_NAME_SIZE];
    char name2[QUEUE_NAME_SIZE];
    int is_creator;
} Chat;

int chat_open(Chat *chat, const char *base_name);
int chat_run(const Chat *chat, volatile sig_atomic_t *stop_requested);
void chat_close(Chat *chat);

#endif
