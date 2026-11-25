#ifndef HEADER_H
#define HEADER_H

int send_files(int data_fd, int ready_fd, int file_count, char *files[]);
int receive_files(int data_fd, int ready_fd);

#endif
