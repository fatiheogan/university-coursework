CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET  = procSearch

SRCS    = main.c signals.c pattern.c dirutils.c worker.c output.c
OBJS    = $(SRCS:.c=.o)
HDRS    = types.h signals.h pattern.h dirutils.h worker.h output.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
