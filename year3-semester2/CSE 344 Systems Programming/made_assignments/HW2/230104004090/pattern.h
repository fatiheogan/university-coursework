#ifndef PATTERN_H
#define PATTERN_H

/*
 * Custom pattern matching — supports only the '+' operator.
 * '+' means ONE OR MORE of the character immediately before it.
 * Matching is case-insensitive.
 */

/* Internal recursive helper — matches pattern against str from current position */
int do_match(const char *pattern, const char *str);

/* Returns 1 if pattern matches anywhere */
int pattern_match(const char *pattern, const char *str);

#endif /* PATTERN_H */
