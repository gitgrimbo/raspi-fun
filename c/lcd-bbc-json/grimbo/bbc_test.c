#include "bbc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    json_t *root = NULL;
    json_error_t error;
    json_t *int_ob = NULL;
    json_t *broadcasts = NULL;
    double page;
    double total;
    double offset;

    root = grimbo_bbc_get_upcoming_science_and_nature_from_url(&error);
    if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 1;
    }

    int_ob = json_object_get(root, "page");
    if (!json_is_integer(int_ob))
    {
        fprintf(stderr, "error: page is not an integer\n");
        return 1;
    }

    page = json_number_value(int_ob);
    printf("page=%f\n", page);

    int_ob = json_object_get(root, "total");
    if (!json_is_integer(int_ob))
    {
        fprintf(stderr, "error: total is not an integer\n");
        return 1;
    }

    total = json_number_value(int_ob);
    printf("total=%f\n", total);

    int_ob = json_object_get(root, "offset");
    if (!json_is_integer(int_ob))
    {
        fprintf(stderr, "error: offset is not an integer\n");
        return 1;
    }

    offset = json_number_value(int_ob);
    printf("offset=%f\n", offset);

    broadcasts = json_object_get(root, "broadcasts");
    if (!json_is_array(broadcasts))
    {
        fprintf(stderr, "error: broadcasts is not an array\n");
        return 1;
    }

    printf("broadcasts.length=%d", json_array_size(broadcasts));

    json_decref(root);
    return 0;
}
