#include "bbc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <jansson.h>

json_t * handle_json_text (char * text, json_error_t * error) {
    json_t *root;

    root = json_loads(text, 0, error);

    return root;
}

// CURL functions
// See http://curl.haxx.se/libcurl/c/getinmemory.html

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        /* out of memory! */ 
        printf("not enough memory (realloc returned NULL)\n");
        // GRIMBO
        // TODO
        // Change here. Return -1, don't call exit().
        // Is this ok?
        return (size_t)-1;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

json_t * grimbo_bbc_get_upcoming_science_and_nature_from_url (json_error_t * error) {
    json_t *root = NULL;

    CURL *curl_handle;
  
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunk.size = 0;    /* no data at this point */ 

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */ 
    curl_handle = curl_easy_init();

    /* specify URL to get */ 
    curl_easy_setopt(curl_handle, CURLOPT_URL, "http://www.bbc.co.uk/tv/programmes/genres/factual/scienceandnature/schedules/upcoming.json");

    /* send all data to this function  */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */ 
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */ 
    curl_easy_perform(curl_handle);

    /* cleanup curl stuff */ 
    curl_easy_cleanup(curl_handle);

    printf("%lu bytes retrieved\n", (long)chunk.size);

    if (chunk.memory) {
        root = handle_json_text(chunk.memory, error);
        if (!root) {
            fprintf(stderr, "error: on line %d: %s\n", error->line, error->text);
            fprintf(stderr, "%s\n", chunk.memory);
        }
        free(chunk.memory);
        chunk.memory = NULL;
    }

      /* we're done with libcurl, so clean it up */ 
    curl_global_cleanup();

    return root;
}
