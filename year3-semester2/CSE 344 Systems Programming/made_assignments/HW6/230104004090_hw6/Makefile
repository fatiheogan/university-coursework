CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

all: hogwarts wizard professor

hogwarts: hogwarts.c common.c common.h
	$(CC) $(CFLAGS) -o hogwarts hogwarts.c common.c

wizard: wizard.c common.c common.h
	$(CC) $(CFLAGS) -o wizard wizard.c common.c

professor: professor.c common.c common.h
	$(CC) $(CFLAGS) -o professor professor.c common.c

clean:
	rm -f hogwarts wizard professor *.o

.PHONY: all clean
