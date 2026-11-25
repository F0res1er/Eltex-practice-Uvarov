CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Wpedantic -D_POSIX_C_SOURCE=200809L

TARGET = code

SRCS = main.c source.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ -lm

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)