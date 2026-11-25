#include "header.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char **files = malloc((size_t)argc * sizeof(*files));
    const char *fifo_name = NULL;
    char *ready_name = NULL;
    int file_count = 0;
    int data_pipe[2], ready_pipe[2];
    int data_fd, ready_fd;
    int result;
    pid_t pid;

    if (files == NULL) 
    {
        perror("malloc");
        return EXIT_FAILURE;
    }

    // Обработка аргументов ввода
    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], "-p") == 0) 
        {
            if (fifo_name != NULL || i + 1 >= argc) 
            {
                fprintf(stderr, "Неверное использование ключа -p\n");
                free(files);
                return EXIT_FAILURE;
            }

            fifo_name = argv[++i];
        } 

        else 
        {
            files[file_count++] = argv[i];
        }
    }

    if (file_count == 0) 
    {
        fprintf(stderr, "Использование: %s [-p имя_канала] файл...\n", argv[0]);
        free(files);
        return EXIT_FAILURE;
    }

    // Создание каналов
    if (fifo_name != NULL)
    {
        ready_name = malloc(strlen(fifo_name) + sizeof(".ready"));
        
        if (ready_name == NULL) 
        {
            perror("malloc");
            free(files);
            return EXIT_FAILURE;
        }

        sprintf(ready_name, "%s.ready", fifo_name);

        if (mkfifo(fifo_name, 0600) == -1) 
        {
            perror("mkfifo");
            free(ready_name);
            free(files);
            return EXIT_FAILURE;
        }

        if (mkfifo(ready_name, 0600) == -1) 
        {
            perror("mkfifo");
            unlink(fifo_name);
            free(ready_name);
            free(files);
            return EXIT_FAILURE;
        }
    } 
    
    else 
    {
        if (pipe(data_pipe) == -1) 
        {
            perror("pipe");
            free(files);
            return EXIT_FAILURE;
        }

        if (pipe(ready_pipe) == -1) 
        {
            perror("pipe");
            close(data_pipe[0]);
            close(data_pipe[1]);
            free(files);
            return EXIT_FAILURE;
        }
    }

    signal(SIGPIPE, SIG_IGN);

    // Создание дочернего процесса
    pid = fork();

    if (pid == -1) 
    {
        perror("fork");

        if (fifo_name == NULL) 
        {
            close(data_pipe[0]);
            close(data_pipe[1]);
            close(ready_pipe[0]);
            close(ready_pipe[1]);
        } 
        else 
        {
            unlink(fifo_name);
            unlink(ready_name);
            free(ready_name);
        }

        free(files);
        return EXIT_FAILURE;
    }

    // Открытие нужных концов каналов
    if (fifo_name != NULL) 
    {
        if (pid == 0) 
        {
            data_fd = open(fifo_name, O_RDONLY);
            ready_fd = open(ready_name, O_WRONLY);
        } 
        
        else 
        {
            data_fd = open(fifo_name, O_WRONLY);
            ready_fd = open(ready_name, O_RDONLY);
        }
    } 
    
    else if (pid == 0) 
    {
        close(data_pipe[1]);
        close(ready_pipe[0]);

        data_fd = data_pipe[0];
        ready_fd = ready_pipe[1];
    } 
    
    else 
    {
        close(data_pipe[0]);
        close(ready_pipe[1]);

        data_fd = data_pipe[1];
        ready_fd = ready_pipe[0];
    }

    // Работа родительского и дочернего процессов
    if (data_fd == -1 || ready_fd == -1) 
    {
        perror("open");
        result = EXIT_FAILURE;
    } 
    
    else if (pid == 0) 
    {
        result = receive_files(data_fd, ready_fd);
    } 
    
    else 
    {
        result = send_files(data_fd, ready_fd, file_count, files);
    }

    if (data_fd != -1) close(data_fd);
    if (ready_fd != -1) close(ready_fd);

    if (pid == 0) 
    {
        _exit(result);
    }

    // Ожидание завершения дочернего процесса
    int status;

    if (waitpid(pid, &status, 0) == -1) 
    {
        perror("waitpid");
        result = EXIT_FAILURE;
    } 
    
    else if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) 
    {
        result = EXIT_FAILURE;
    }

    // Удаление именованных каналов
    if (fifo_name != NULL) 
    {
        unlink(fifo_name);
        unlink(ready_name);
    }

    free(ready_name);
    free(files);

    return result;
}
