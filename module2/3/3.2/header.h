#ifndef HEADER_H
#define HEADER_H

#include <stdint.h>

uint32_t ip_to_uint(const char *ip);
int parse_mask(const char *text, uint32_t *mask);
uint32_t generate_ip();

void print_packet(int number, uint32_t ip, int own);
void print_stats(int own, int total);

#endif