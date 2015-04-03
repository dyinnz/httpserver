#pragma once

#include <cstring>

class Request;

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

char *CreateResponse(char *p, const char *end, const Request &request);
char *AppendStatusLine(char *p, int status);
char *AppendHeader(char *p, const char *keys, const char *values);
char *AppendEntity(char *p, const char *data, size_t size);
//char *AppendEntityBody(char *p, const char *data);
//

class Response {
public:
    size_t data_size {0};
    int status {0};

    char *header {NULL};
    char *body {NULL};
};





