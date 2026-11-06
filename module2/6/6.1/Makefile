
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

TARGET = code
LIB = libcontacts.a

.PHONY: all clean

all: $(TARGET)

$(LIB): back.o
	ar rcs $@ $^

$(TARGET): main.o front.o $(LIB)
	$(CC) main.o front.o $(LIB) -o $@

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main.o front.o back.o $(LIB) $(TARGET)