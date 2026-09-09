#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <string.h>
#include "pattern.h"

/* Recursive case-insensitive match from current positions in pattern and string */
int do_match(const char *p, const char *s)
{
    /* Pattern fully consumed -> match succeeded */
    if (*p == '\0') return 1;

    if (*(p + 1) == '+') {
        /* p[0]+ : need at least one matching char, then try rest of pattern */
        if (!*s || tolower((unsigned char)*s) != tolower((unsigned char)*p))
            return 0;   /* no match for mandatory first occurrence */
        s++;            /* consume first occurrence */
        for (;;) {
            if (do_match(p + 2, s)) return 1;
            /* consume extra occurrences of the same char */
            if (!*s || tolower((unsigned char)*s) != tolower((unsigned char)*p))
                break;
            s++;
        }
        return 0;
    }

    /* Normal character: must match exactly (case-insensitive) */
    if (!*s || tolower((unsigned char)*s) != tolower((unsigned char)*p))
        return 0;
    return do_match(p + 1, s + 1);
}

/* Tries do_match at every position in str, returns 1 if any position matches */
int pattern_match(const char *pattern, const char *str)
{
    int n = (int)strlen(str);
    for (int i = 0; i <= n; i++)
        if (do_match(pattern, str + i))
            return 1;
    return 0;
}
