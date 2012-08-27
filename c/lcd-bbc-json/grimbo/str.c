#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Returns a pointer to the last occurrence of s in src, or NULL.
 */
char * grimbo_str_last_index_of( const char *src, const char *s ) {
    char * found = strstr( src, s );
    char * pos = NULL;
    while (found != NULL) {
        pos = (char *)(found - src);
        found = strstr( found+1, s );
    }
    return pos;
}
