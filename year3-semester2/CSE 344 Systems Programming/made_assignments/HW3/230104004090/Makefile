CC      = gcc
CFLAGS  = -Wall -Wextra -g -O0 -D_GNU_SOURCE
LDFLAGS = -lrt -lpthread

TARGET  = hw3
SRCS    = main.c shared.c word_carrier.c letter_carrier.c sorting.c elevator.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c shared.h utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

# Quick test run
run: $(TARGET)
	./$(TARGET) -f 4 -w 2 -l 2 -s 2 -c 5 -d 10 -r 10 -i input.txt -o output.txt
