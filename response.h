#pragma once

#include <cstring>

class Request;

class Response {
public:
    ~Response() {
        if (body) delete[] body;
    }
    size_t data_size {0};
    size_t header_size {0};
    int status {0};

    char *header {NULL};
    char *body {NULL};
};

inline char *copy_move(char *dest, const char *source, size_t n) {
    return strncpy(dest, source, n) + n;
}

inline char *copy_move(char *dest, const char *source) {
    size_t n = strlen(source);
    return strncpy(dest, source, n) + n;
}

inline char *AppendCRLF(char *p) {
    *p++ = '\r';
    *p++ = '\n';
    return p;
}

char *CreateResponseHeader(char *p, const char *end, Response &response);
char *AppendStatusLine(char *p, int status);
char *AppendHeader(char *p, const char *keys, const char *values);
char *AppendEntity(char *p, const char *data, size_t size);
char *AppendEntityHeader(char *p, size_t data_size);
//


