CC      = gcc
CFLAGS  = -Wall -Wextra -pthread -g
LDFLAGS = -pthread

TARGET  = analyzer

SRCS    = main.c shm.c reader.c dispatcher.c analyzer.c aggregator.c watchdog.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
