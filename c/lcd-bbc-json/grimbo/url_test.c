#include "url.h"
#include "str.h"

// grimbo_bo, included to check pclose() result.
// errno is thread-local, so thread safe.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * grimbo_url_get_test_filename () {
    char * filename = "synd.txt";
    char * home = getenv("HOME");
    // +1 for path separator and for termination
    int full_filename_len = strlen(home) + 1 + strlen(filename) + 1;
    char *const full_filename = malloc(full_filename_len);
    snprintf(full_filename, full_filename_len, "%s%s%s", home, "/", filename);
    return full_filename;
}

void grimbo_url_test_buffer (char * buffer) {
    printf("%s\n", buffer);
}

int main(int argc, char **argv) {
    /*
    char * full_filename = grimbo_url_get_test_filename();
    printf("filename=%s\n", full_filename);

    int res = grimbo_url_url_to_file("http://www.paulgri.me/syndicate/get", full_filename);
    if (res < 0) {
        printf("error");
        return 0;
    }
    */

    char * buffer = NULL;
    buffer = grimbo_url_url_to_buffer("http://www.paulgri.me/syndicate/get");
    printf("buffer=%d\n", buffer);
    if (buffer) {
        grimbo_url_test_buffer(buffer);
        free(buffer);
        buffer = NULL;
    }

    //free(full_filename);
    //full_filename = NULL;
}
