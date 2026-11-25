#include "header.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int chat_open(Chat *chat, const char *base_name)
{
    if (base_name[0] != '/' || base_name[1] == '\0' || strchr(base_name + 1, '/') != NULL) 
    {
        fprintf(stderr, "Имя очереди должно иметь вид /{имя}\n");
        return -1;
    }

    int len1 = snprintf(chat->name1, sizeof(chat->name1), "%s_1", base_name);
    int len2 = snprintf(chat->name2, sizeof(chat->name2), "%s_2", base_name);

    if (len1 < 0 || len2 < 0 ||
        (size_t)len1 >= sizeof(chat->name1) ||
        (size_t)len2 >= sizeof(chat->name2)) 
    {
        fprintf(stderr, "Слишком длинное имя очереди\n");
        return -1;
    }

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MESSAGE_SIZE;

    chat->is_creator = 0;

    chat->queue1 = mq_open(chat->name1, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0600, &attr);

    if (chat->queue1 != (mqd_t)-1) 
    {
        chat->is_creator = 1;
    } 
    
    else if (errno == EEXIST) 
    {
        chat->queue1 = mq_open(chat->name1, O_RDWR | O_NONBLOCK);
    }

    if (chat->queue1 == (mqd_t)-1) 
    {
        perror("mq_open (очередь 1)");
        return -1;
    }

    chat->queue2 = mq_open(chat->name2, O_CREAT | O_RDWR | O_NONBLOCK, 0600, &attr);

    if (chat->queue2 == (mqd_t)-1) 
    {
        perror("mq_open (очередь 2)");
        mq_close(chat->queue1);

        if (chat->is_creator) 
        {
            mq_unlink(chat->name1);
        }

        return -1;
    }

    struct mq_attr existing;

    if (mq_getattr(chat->queue1, &existing) == -1 ||
        existing.mq_msgsize != MESSAGE_SIZE ||
        mq_getattr(chat->queue2, &existing) == -1 ||
        existing.mq_msgsize != MESSAGE_SIZE) 
    {
        fprintf(stderr, "Ошибка: неподходящие атрибуты очередей\n");
        chat_close(chat);
        return -1;
    }

    printf("Роль: %s\n", chat->is_creator ? "Создатель" : "Пользователь");

    return 0;
}

void chat_close(Chat *chat)
{
    if (mq_close(chat->queue1) == -1) 
    {
        perror("mq_close (очередь 1)");
    }

    if (mq_close(chat->queue2) == -1) 
    {
        perror("mq_close (очередь 2)");
    }

    if (chat->is_creator) 
    {
        if (mq_unlink(chat->name1) == -1) 
        {
            perror("mq_unlink (очередь 1)");
        }

        if (mq_unlink(chat->name2) == -1) 
        {
            perror("mq_unlink (очередь 2)");
        }
    }
}

int chat_run(const Chat *chat, volatile sig_atomic_t *stop_requested)
{
    mqd_t receive_queue = chat->is_creator ? chat->queue1 : chat->queue2;
    mqd_t send_queue = chat->is_creator ? chat->queue2 : chat->queue1;

    struct pollfd fds[2] = {
        {.fd = STDIN_FILENO, .events = POLLIN},
        {.fd = (int)receive_queue, .events = POLLIN}
    };

    setvbuf(stdin, NULL, _IONBF, 0);

    printf("Введите сообщение (/exit — выход):\n> ");
    fflush(stdout);

    while (*stop_requested) 
    {
        int ready = poll(fds, 2, -1);

        if (ready == -1) 
        {
            if (errno == EINTR) continue;

            perror("poll");
            return -1;
        }

        if (ready == 0) continue;

        // Получение сообщения от собеседника
        if (fds[1].revents & POLLIN) 
        {
            char message[MESSAGE_SIZE + 1];
            unsigned priority;

            ssize_t size = mq_receive(receive_queue, message, MESSAGE_SIZE, &priority);

            if (size == -1) 
            {
                if (errno == EAGAIN || errno == EINTR) continue;
                
                perror("mq_receive");
                return -1;
            }

            if (priority == EXIT_PRIORITY) 
            {
                printf("\nСобеседник завершил чат.\n");
                return 0;
            }

            message[size] = '\0';
            printf("\nСобеседник: %s\n> ", message);
            fflush(stdout);
        }

        // Ввод и отправка сообщения
        if (fds[0].revents & POLLIN) 
        {
            char message[MESSAGE_SIZE + 2];

            if (fgets(message, sizeof(message), stdin) == NULL) 
            {
                if (ferror(stdin) && errno == EINTR) 
                {
                    clearerr(stdin);
                    continue;
                }

                break;
            }

            size_t length = strcspn(message, "\n");

            if (message[length] != '\n' && length >= MESSAGE_SIZE) 
            {
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);
            }

            message[length] = '\0';

            if (length >= MESSAGE_SIZE) 
            {
                fprintf(stderr, "Сообщение слишком длинное (максимум %d символов)\n", MESSAGE_SIZE - 1);
            } 
            
            else if (strcmp(message, "/exit") == 0) break;
            
            else if (length > 0 && mq_send(send_queue, message, length + 1, MESSAGE_PRIORITY) == -1) 
            {
                perror("mq_send");
            }

            printf("> ");
            fflush(stdout);
        }

        if (fds[0].revents & (POLLHUP | POLLERR | POLLNVAL)) break;

        if (fds[1].revents & (POLLERR | POLLNVAL)) 
        {
            fprintf(stderr, "Ошибка при ожидании входящих сообщений\n");
            return -1;
        }
    }

    // Уведомление собеседника о завершении работы
    if (mq_send(send_queue, "exit", sizeof("exit"), EXIT_PRIORITY) == -1) 
    {
        perror("mq_send (завершение)");
        return -1;
    }

    printf("Чат завершён.\n");
    return 0;
}