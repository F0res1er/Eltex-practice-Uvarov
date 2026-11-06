CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

TARGET = code
LIB = libcontacts.so

.PHONY: all clean

all: $(TARGET)

$(LIB): back.o
	$(CC) -shared $^ -o $@

$(TARGET): main.o front.o $(LIB)
	$(CC) main.o front.o -L. -lcontacts -o $@

back.o: back.c header.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main.o front.o back.o $(LIB) $(TARGET)