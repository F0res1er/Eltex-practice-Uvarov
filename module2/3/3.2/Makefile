
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

TARGET = code

SRCS = main.c back.c front.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ -lm

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

test:
	python3 -m unittest discover -s tests -p 'test_*.py' -v

clean:
	rm -f $(OBJS) $(TARGET)