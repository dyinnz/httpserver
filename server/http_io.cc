#include <cstring>
#include "unistd.h" // read() / write()

#include "http_io.h"
#include "utility.h"

size_t ReadRequestHeader(int sockfd, char *buff, size_t buff_size) {
    // TODO:
    return read(sockfd, buff, buff_size);
}

size_t ReadRequestBody(int sockfd, char *buff, size_t read_size) {
    size_t readn = read(sockfd, buff, read_size);
    if (readn < read_size) {
        http_debug("The size of the data read from sock is smaller than the required size!");
    }
    return readn;
}

size_t SendSocketData(int sockfd, const char *buff, size_t write_size) {
    // http_debug("before write!\n");

    // TODO: There is a bug, after client sent data but closed soon afterwards,
    // kernel will sende a signal SIGPIPE[13] to process, which the process would
    // be terminate because 'Write to pipe with no readers' without handling.

    size_t writen = write(sockfd, buff, write_size);
    // http_debug("after write!\n");
    if (writen < write_size) {
        http_debug("The size of the data writen into sock is smaller than the required size!");
    }
    return writen;
}

char *ReadFileData(const char *path, size_t *read_size) {
    FILE *pfile = fopen(path, "r");
    if (!pfile) {
        http_debug("Open file error\n");
        return NULL;
    }

    fseek(pfile, 0, SEEK_END);
    *read_size = ftell(pfile);
    rewind(pfile);

    char *buff = static_cast<char*>( http_alloc(*read_size) );

    http_debug("The size of file: %d\n", *read_size);
    // size_t readn {0};
    if (1 != fread(buff, *read_size, 1, pfile)) {
        *read_size = 0;
        http_free(buff);
        fclose(pfile);
        return NULL;

    } else {
        fclose(pfile);
        return buff;
    }
}

char *ReadFileData(const strpair &sp_path, size_t *read_size) {
    char str_path[sp_path.length()] = {0};
    // TODO:
    strncpy(str_path, sp_path.beg()+1, sp_path.length()-1);
    http_debug("%s\n", str_path);
    return ReadFileData(str_path, read_size);
}

bool IsFileExist(const char *path) {
    FILE *pfile = fopen(path, "r");
    bool ret = (NULL != pfile);
    fclose(pfile);
    return ret;
}

bool IsFileExist(const strpair &sp_path) {
    char str_path[sp_path.length()] = {0};
    // TODO:
    strncpy(str_path, sp_path.beg()+1, sp_path.length()-1);
    http_debug("%s\n", str_path);
    return IsFileExist(str_path); 
}

