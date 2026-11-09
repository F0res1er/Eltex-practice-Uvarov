#include "header.h"
#include <stdio.h>
#include <arpa/inet.h>

void print_packet(int number, uint32_t ip, int own) 
{
    struct in_addr address;
    char text[INET_ADDRSTRLEN];

    address.s_addr = htonl(ip); // из порядка ПК в сетевой порядок
    inet_ntop(AF_INET, &address, text, sizeof(text)); //бинарный ip в текстовый

    printf("%d. %s -> %s\n", number, text, own ? "своя подсеть" : "другая сеть");
}

void print_stats(int own, int total) 
{
    printf("\nСвоя подсеть: %d (%.2f%%)\n", own, 100.0 * own / total);

    printf("Другие сети: %d (%.2f%%)\n", total - own, 100.0 * (total - own) / total);
}