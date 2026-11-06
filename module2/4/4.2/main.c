#include "header.h"
#include <stdlib.h>
#include <time.h>

int main(void)
{
    Queue queue = {NULL, NULL};
    Message result;

    srand((unsigned int)time(NULL));
    run(&queue);

    while (dequeue(&queue, FIRST, 0, &result)) {
    }

    return 0;
}
