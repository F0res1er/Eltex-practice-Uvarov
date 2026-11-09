#include "header.h"
#include <stdlib.h>
#include <arpa/inet.h>

uint32_t ip_to_uint(const char *ip) 
{
    struct in_addr address;
    inet_pton(AF_INET, ip, &address); //преобразуем текстовый ip в двоичную форму
    return ntohl(address.s_addr); //перевод из сетевого порядка в порядок ПК
}

int parse_mask(const char *text, uint32_t *mask) 
{
    if (text[0] == '/') 
    {
        char *end;
        long bits = strtol(text + 1, &end, 10);

        if (end == text + 1 || *end != '\0' || bits < 0 || bits > 32) return 0;

        *mask = bits == 0 ? 0 : UINT32_MAX << (32 - bits);
    } 
    
    else 
    {
        struct in_addr address;

        if (inet_pton(AF_INET, text, &address) != 1)
            return 0;

        *mask = ntohl(address.s_addr);
    }

    uint32_t inverted = ~(*mask);
    return (inverted & (inverted + 1)) == 0;
}

uint32_t generate_ip(void) 
{
    return ((uint32_t)(rand() & 255) << 24) | //генерим первый октет
           ((uint32_t)(rand() & 255) << 16) | //генерим второй октет
           ((uint32_t)(rand() & 255) << 8)  | //генерим третий октет
           (uint32_t)(rand() & 255); //генерим четвёртый октет и побитовым И их объединяем
}