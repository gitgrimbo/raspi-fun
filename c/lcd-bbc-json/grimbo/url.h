#include <stdio.h>

FILE * grimbo_url_wget(char * url);
int grimbo_url_copy(FILE * src, FILE * dest);
int grimbo_url_url_to_file_descriptor(char * url, FILE * fp);
int grimbo_url_url_to_file(char * url, char * filename);
char * grimbo_url_url_to_buffer(char * url);
char * grimbo_url_read_file(char * filename);
