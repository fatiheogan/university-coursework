#define _POSIX_C_SOURCE 200809L
#include "common.h"

/* Keep writing until all bytes are sent */
int send_all(int fd, const char *buf, size_t len)
{
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, buf + sent, len - sent);
        if (n < 0) { if (errno == EINTR) continue; return -1; }
        if (n == 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}

/* Append a newline and send the whole line */
int send_line(int fd, const char *msg)
{
    char buf[MAX_LINE + 2];
    int len = snprintf(buf, sizeof(buf), "%s\n", msg);
    if (len < 0) return -1;
    return send_all(fd, buf, (size_t)len);
}

/* Format the current local time as a log timestamp */
void get_timestamp(char *buf, size_t n)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, n, "[%Y-%m-%d %H:%M:%S]", tm);
}

/* Strip trailing '\r' and '\n' characters */
void trim_crlf(char *s)
{
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\r' || s[len-1] == '\n'))
        s[--len] = '\0';
}

/* Convert a string to a non-negative int, -1 if invalid */
int parse_int(const char *str)
{
    if (!str || *str == '\0') return -1;
    char *end;
    long v = strtol(str, &end, 10);
    if (*end != '\0' || v < 0 || v > 2147483647) return -1;
    return (int)v;
}
