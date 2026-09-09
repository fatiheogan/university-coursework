#ifndef COMMON_H
#define COMMON_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

/* Protocol and buffer size limits */
#define MAX_LINE             512
#define MAX_USERNAME         32
#define MAX_INGREDIENT_NAME  17
#define MAX_INGREDIENTS      256
#define MAX_CLIENTS_HARD     1024

/* One ingredient: name and current quantity */
typedef struct {
    char name[MAX_INGREDIENT_NAME];
    int  quantity;
} ingredient_t;

/* Write all bytes, handling partial writes */
int  send_all(int fd, const char *buf, size_t len);

/* Send one message followed by a newline */
int  send_line(int fd, const char *msg);

/* Build a "[YYYY-MM-DD HH:MM:SS]" timestamp string */
void get_timestamp(char *buf, size_t n);

/* Remove trailing CR/LF characters */
void trim_crlf(char *s);

/* Parse a non-negative integer, return -1 on error */
int  parse_int(const char *str);

#endif /* COMMON_H */
