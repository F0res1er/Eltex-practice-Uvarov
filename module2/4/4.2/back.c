#include "header.h"
#include <stdlib.h>
#include <stdio.h>

int enqueue(Queue *queue, int id, unsigned char priority)
{
    Message *message = malloc(sizeof(*message));
    if (message == NULL)
        return 0;

    message->id = id;
    message->priority = priority;
    message->next = NULL;

    if (queue->tail != NULL)
        queue->tail->next = message;
    
    else
        queue->head = message;

    queue->tail = message;
    return 1;
}

int dequeue(Queue *queue, ExtractMode mode, unsigned char priority, Message *result)
{
    Message *current = queue->head;
    Message *previous = NULL;
    Message *selected = NULL;
    Message *selected_prev = NULL;

    while (current != NULL) 
    {
        if (mode == FIRST || (mode == EXACT && current->priority == priority)) 
        {
            selected = current;
            selected_prev = previous;
            break;
        }

        if (mode == NOT_LOWER && current->priority >= priority && (selected == NULL || current->priority < selected->priority)) 
        {
            selected = current;
            selected_prev = previous;
        }

        previous = current;
        current = current->next;
    }

    if (selected == NULL)
        return 0;

    if (selected_prev != NULL)
        selected_prev->next = selected->next;
    
    else
        queue->head = selected->next;

    if (queue->tail == selected)
        queue->tail = selected_prev;

    result->id = selected->id;
    result->priority = selected->priority;
    free(selected);
    return 1;
}

/*void printQueue(const Queue *queue)
{
    Message *current = queue->head;

    if (current == NULL) 
    {
        printf("Очередь пуста.\n");
        return;
    }

    printf("\nТекущая очередь:\n");

    while (current != NULL) 
    {
        printf("Сообщение %d, приоритет %u\n",
               current->id, (unsigned int)current->priority);
        current = current->next;
    }
}*/

void printQueue(const Queue *queue)
{
    Message *current = queue->head;
    int priority;

    for(priority = 255; priority >= 0; priority--)
    {
        current = queue->head;
        while (current != NULL) 
        {
            if (current->priority == priority)
            {
                printf("Сообщение %d, приоритет %u\n", current->id, (unsigned int)current->priority);
            }
            current = current->next;
        }
    }
}