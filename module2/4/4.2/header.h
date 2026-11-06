
#ifndef HEADER_H
#define HEADER_H

typedef struct Message {
    int id;
    unsigned char priority;
    struct Message *next;
} Message;

typedef struct {
    Message *head;
    Message *tail;
} Queue;

typedef enum {
    FIRST = 1,
    EXACT,
    NOT_LOWER
} ExtractMode;

int enqueue(Queue *queue, int id, unsigned char priority);
int dequeue(Queue *queue, ExtractMode mode, unsigned char priority, Message *result);
void run(Queue *queue);
void printQueue(const Queue *queue);

#endif