CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

TARGET = code

SRCS = main.c back.c front.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ -lm

%.o: %.c header2.2.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)