#pragma once

#include <cstdio>

#include "utility.h"

inline bool is_exist(const char *filename) {
    return NULL != fopen(filename, "r");
}

class Response;

class HTTPFile {
public:
    HTTPFile(const strpair &sp) {
        char path[sp.length()+2] {0};
        if ('/' == *sp.beg()) {
            path[0] = '.';
            strncpy(path+1, sp.beg(), sp.length());
        } else {
            strncpy(path, sp.beg(), sp.length());
        }
        http_logn(path);        
        pfile_ = fopen(path, "r");
    }
    HTTPFile(const char *filename) {
        pfile_ = fopen(filename, "r");
    }

    ~HTTPFile() {
        if (pfile_) fclose(pfile_);
        if (buff_) delete[] buff_;
    }

    size_t size() {
        fseek(pfile_, 0, SEEK_END);
        size_t file_size = ftell(pfile_);
        return file_size;
    }

    const char *read(size_t *pread = NULL) {
        if (!pfile_) return NULL;

        if (!buff_) {
            size_t file_size = size();
            buff_ = new char[file_size];
            fseek(pfile_, 0, SEEK_SET);
            file_size = fread(buff_, file_size, 1, pfile_);

            if (pread) *pread = file_size;
        }
        return buff_;
    }

    bool read(Response &response);

private:
    // Don't support
    HTTPFile& operator=(const HTTPFile&);
    HTTPFile(const HTTPFile&);

    FILE *pfile_ {NULL};
    char *buff_ {NULL};
};
