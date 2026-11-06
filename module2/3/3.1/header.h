#ifndef HEADER_H
#define HEADER_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

void menu();
int read_line(char *s, size_t size);
void show_rights(mode_t mode);

int parse_rights(const char *s, mode_t *mode);
int file_rights(const char *name, mode_t *mode);
int change_rights(mode_t *mode, const char *cmd);

#endif