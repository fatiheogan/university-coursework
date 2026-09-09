#ifndef DIRUTILS_H
#define DIRUTILS_H

#include "types.h"

/* Returns an array of immediate subdirectory paths under root; sets count */
char **get_subdirs(const char *root, int *count);

/* Searches dir recursively without forking (used when no subdirs exist) */
void direct_search(const char  *dir,
                   const char  *pattern,
                   long         min_size,
                   MatchResult *results,
                   int         *result_cnt,
                   int         *total_scan);

#endif /* DIRUTILS_H */
