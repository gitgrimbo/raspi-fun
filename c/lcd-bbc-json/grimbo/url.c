#include "url.h"
#include "str.h"

// grimbo_bo, included to check pclose() result.
// errno is thread-local, so thread safe.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Opens a file pointer to wget for the provided url.
 */
FILE * grimbo_url_wget(char * url) {
    printf("grimbo_url_wget(%s)\n", url);

    FILE *fp;
    int CMD_MAX = 512;
    char cmd[CMD_MAX+1];

    // wget to the console.
    snprintf(cmd, CMD_MAX+1, "wget -qO- %s", url);
    printf("cmd=%s\n", cmd);

    fp = popen(cmd, "r");
    if (!fp) {
        printf("error calling %s\n", cmd);
        return NULL;
    }

    return fp;
}

/**
 * Copies from src stream to dest stream.
 */
int grimbo_url_copy(FILE * src, FILE * dest) {
    printf("grimbo_url_copy(%d, %d)\n", src, dest);

    int CONTENT_MAX = 1024*10;
    char content[CONTENT_MAX+1];
    int written = 0;
    int len = 0;
    while (fgets(content, CONTENT_MAX, src)) {
        //printf("%s\n", content);
        len = strlen(content);
        printf("read %d bytes\n", len);
        fprintf(dest, content);
        written += len;
    }
    printf("written %d bytes\n", written);
    return written;
}

/**
 * Downloads the url to the destination file pointer.
 */
int grimbo_url_url_to_file_descriptor(char * url, FILE * fp) {
    printf("grimbo_url_url_to_file_descriptor(%s, %d)\n", url, fp);

    FILE *wget_output;
    int status;
    int written;

    wget_output = grimbo_url_wget(url);
    if (!wget_output) {
        printf("error calling wget for %s\n", url);
        return -1;
    }

    if (!fp) {
        printf("File descriptor is NULL\n");
        return -1;
    }

    written = grimbo_url_copy(wget_output, fp);
    fclose(fp);

    printf("written=%d\n", written);

    status = pclose(wget_output);
    printf("status=%d\n", status);

    if (status == -1) {
        printf("errno=%d\n", errno);
        return -1;
    }

    // wait for child to exit, and store child's exit status.
    wait(&status);
    printf("Child exit code: %d\n", WEXITSTATUS(status));

    return status;
}

/**
 * Downloads the url to the destination file pointer.
 */
int grimbo_url_url_to_file(char * url, char * filename) {
    printf("grimbo_url_url_to_file(%s, %s)\n", url, filename);

    FILE * dest_output;

    dest_output = fopen(filename, "w");
    if (!dest_output) {
        printf("Error opening file for output %s. errno=%d\n", filename, errno);
        return -1;
    }

    printf("dest_output=%d, %s\n", dest_output, filename);

    return grimbo_url_url_to_file_descriptor(url, dest_output);
}

/**
 * Downloads the url and returns contents as a char buffer.
 */
char * grimbo_url_url_to_buffer(char * url) {
    printf("grimbo_url_url_to_buffer(%s)\n", url);

    char filename[] = "/tmp/grimbo_url_XXXXXX";
    int fd;
    FILE * fp;

    fd = mkstemp(filename);
    if (!fd) {
        printf("Could not create temp file %s\n", filename);
        return NULL;
    }

    printf("Using temp file %s\n", filename);
    printf("File descriptor %d\n", fd);

    fp = fdopen(fd, "w");
    if (!fp) {
        printf("Could not create file pointer from file descriptor for file %s\n", filename);
        return NULL;
    }

    int res = grimbo_url_url_to_file_descriptor(url, fp);
    if (res < 0) {
        printf("Could not write URL contents to file %s\n", filename);
        return NULL;
    }

    char * buffer = grimbo_url_read_file(filename);
    remove(filename);
    return buffer;
}

/**
 * Reads the file as a char buffer.
 */
char * grimbo_url_read_file(char * filename) {
    FILE *file;
    char *buffer;
    unsigned long fileLen;

    //Open file
    file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Unable to open file %s\n", filename);
        fprintf(stderr, "errno=%d, %s\n", errno, strerror(errno));
        return NULL;
    }
    
    //Get file length
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);

    //Allocate memory
    buffer=(char *)malloc(fileLen+1);
    if (!buffer) {
        fprintf(stderr, "Memory error!");
        fclose(file);
        return NULL;
    }

    //Read file contents into buffer
    fread(buffer, fileLen, 1, file);
    fclose(file);

    return buffer;
}
