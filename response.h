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

char *CreateResponseHeader(char *p, const char *end, Request &request);
char *AppendStatusLine(char *p, Request &request);
char *AppendHeader(char *p, const char *keys, const char *values);
char *AppendEntity(char *p, const char *data, size_t size);
char *AppendEntityHeader(char *p, size_t data_size);
//
