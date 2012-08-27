#include "url.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

json_t * handle_json_text (char * text, json_error_t * error) {
    json_t *root;

    root = json_loads(text, 0, error);

    return root;
}

json_t * grimbo_bbc_get_upcoming_science_and_nature_from_url (json_error_t * error) {
    char * buffer = NULL;
    json_t *root = NULL;

    buffer = grimbo_url_url_to_buffer("http://www.bbc.co.uk/tv/programmes/genres/factual/scienceandnature/schedules/upcoming.json");
    printf("buffer=%d\n", buffer);

    if (buffer) {
        root = handle_json_text(buffer, error);
        if (!root) {
            fprintf(stderr, "error: on line %d: %s\n", error->line, error->text);
            fprintf(stderr, "%s\n", buffer);
        }
        free(buffer);
        buffer = NULL;
    }

    return root;
}
