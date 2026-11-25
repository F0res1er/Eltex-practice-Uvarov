#include "header.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BLOCK_SIZE 4096

typedef struct {
    size_t name_length;
    off_t size;
} FileInfo;

// Запись всех указанных байтов
static int write_all(int fd, const void *buffer, size_t count)
{
    const char *data = buffer;

    while (count > 0) 
    {
        ssize_t written = write(fd, data, count);

        if (written == -1 && errno == EINTR) continue;

        if (written <= 0) return -1;

        data += written;
        count -= (size_t)written;
    }

    return 0;
}

// Чтение заданного количества байтов
static int read_all(int fd, void *buffer, size_t count)
{
    char *data = buffer;

    while (count > 0) 
    {
        ssize_t received = read(fd, data, count);

        if (received == -1 && errno == EINTR) continue;

        if (received <= 0) return -1;

        data += received;
        count -= (size_t)received;
    }

    return 0;
}

// Передача данных блоками
static int transfer_data(int input_fd, int output_fd, off_t size)
{
    char buffer[BLOCK_SIZE];

    while (size > 0) 
    {
        size_t chunk = size < BLOCK_SIZE ? (size_t)size : BLOCK_SIZE;
        ssize_t count = read(input_fd, buffer, chunk);

        if (count == -1 && errno == EINTR) continue;

        if (count <= 0 || write_all(output_fd, buffer, (size_t)count) == -1) return -1;

        size -= count;
    }

    return 0;
}

// Родительский процесс: отправка файлов
int send_files(int data_fd, int ready_fd, int file_count, char *files[])
{
    int had_error = 0;

    for (int i = 0; i < file_count; i++)
     {
        int file_fd = open(files[i], O_RDONLY);

        if (file_fd == -1) 
        {
            perror(files[i]);
            had_error = 1;
            continue;
        }

        // Получение размера файла
        struct stat st;

        if (fstat(file_fd, &st) == -1) 
        {
            perror(files[i]);
            close(file_fd);
            had_error = 1;
            continue;
        }

        if (!S_ISREG(st.st_mode)) 
        {
            fprintf(stderr, "%s: не является обычным файлом\n", files[i]);
            close(file_fd);
            had_error = 1;
            continue;
        }

        // Ожидание готовности дочернего процесса
        char ready;
        FileInfo info = {strlen(files[i]) + 1, st.st_size};

        if (read_all(ready_fd, &ready, 1) == -1 ||
            write_all(data_fd, &info, sizeof(info)) == -1 ||
            write_all(data_fd, files[i], info.name_length) == -1 ||
            transfer_data(file_fd, data_fd, info.size) == -1) 
        {
            fprintf(stderr, "Ошибка передачи файла %s\n", files[i]);
            close(file_fd);
            return EXIT_FAILURE;
        }

        close(file_fd);
    }

    // Сообщение об окончании передачи всех файлов
    char ready;
    FileInfo end = {0, 0};

    if (read_all(ready_fd, &ready, 1) == -1 ||
        write_all(data_fd, &end, sizeof(end)) == -1) 
    {
        fprintf(stderr, "Ошибка завершения передачи\n");
        return EXIT_FAILURE;
    }

    return had_error ? EXIT_FAILURE : EXIT_SUCCESS;
}

// Дочерний процесс: получение файлов
int receive_files(int data_fd, int ready_fd)
{
    for (;;) 
    {
        char ready = 'R';
        FileInfo info;

        // Сообщение родителю о готовности
        if (write_all(ready_fd, &ready, 1) == -1 ||
            read_all(data_fd, &info, sizeof(info)) == -1) 
        {
            fprintf(stderr, "Ошибка получения информации о файле\n");
            return EXIT_FAILURE;
        }

        // Нулевая длина имени означает конец передачи
        if (info.name_length == 0)
            return EXIT_SUCCESS;

        // Получение имени файла
        char *name = malloc(info.name_length + 5);

        if (name == NULL) 
        {
            perror("malloc");
            return EXIT_FAILURE;
        }

        if (read_all(data_fd, name, info.name_length) == -1) 
        {
            fprintf(stderr, "Ошибка получения имени файла\n");
            free(name);
            return EXIT_FAILURE;
        }

        // Добавление расширения .copy
        memcpy(name + info.name_length - 1, ".copy", 6);

        // Создание файла-копии
        int file_fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (file_fd == -1)
        {
            perror(name);
            free(name);
            return EXIT_FAILURE;
        }

        // Запись полученных данных
        if (transfer_data(data_fd, file_fd, info.size) == -1)
        {
            fprintf(stderr, "Ошибка записи файла %s\n", name);
            close(file_fd);
            free(name);
            return EXIT_FAILURE;
        }

        close(file_fd);
        free(name);
    }
}