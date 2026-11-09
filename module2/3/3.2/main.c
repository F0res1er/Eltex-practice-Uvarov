#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Использование: %s <IP_шлюза> <маска> <N>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[3]);
    if (n <= 0) 
    {
        printf("N должно быть положительным числом\n");
        return 1;
    }

    uint32_t gateway = ip_to_uint(argv[1]);
    uint32_t mask;

    if (!parse_mask(argv[2], &mask)) 
    {
        printf("Некорректная маска подсети\n");
        return 1;
    }
    
    uint32_t network = gateway & mask; // адрес подсети шлюза 
    int own = 0;

    srand(time(NULL));

    for (int i = 0; i < n; i++) 
    {
        uint32_t ip = generate_ip();
        int is_own = ((ip & mask) == network);

        own += is_own;
        print_packet(i + 1, ip, is_own);
    }

    print_stats(own, n);

    return 0;
}

// 192.1.1.1 255.0.0.0 10000