CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pthread
TARGET  = cargoGTU
SRCS    = main.c queue.c courier.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
