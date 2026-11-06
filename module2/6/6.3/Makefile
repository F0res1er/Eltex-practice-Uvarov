
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

TARGET = code
OBJS = main.o back.o front.o

LIBDIR = libs
LIBS = $(LIBDIR)/libadd.so $(LIBDIR)/libsubtract.so $(LIBDIR)/libmultiply.so $(LIBDIR)/libdivide.so

.PHONY: all clean

all: $(TARGET) $(LIBS)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ -ldl -lm

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(LIBDIR)/libadd.so: back.c header.h | $(LIBDIR)
	$(CC) $(CFLAGS) -fPIC -shared -DLIB_ADD back.c -o $@

$(LIBDIR)/libsubtract.so: back.c header.h | $(LIBDIR)
	$(CC) $(CFLAGS) -fPIC -shared -DLIB_SUBTRACT back.c -o $@

$(LIBDIR)/libmultiply.so: back.c header.h | $(LIBDIR)
	$(CC) $(CFLAGS) -fPIC -shared -DLIB_MULTIPLY back.c -o $@

$(LIBDIR)/libdivide.so: back.c header.h | $(LIBDIR)
	$(CC) $(CFLAGS) -fPIC -shared -DLIB_DIVIDE back.c -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(LIBDIR)
