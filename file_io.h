#pragma once

#include <cstdio>

#include "utility.h"

inline bool is_exist(const char *filename) {
    return NULL != fopen(filename, "r");
}

class HTTPFile {
public:
    HTTPFile(const strpair &sp) {
        char path[sp.length()+1] {0};
        strncpy(path, sp.beg(), sp.length());
        pfile_ = fopen(path, "r");
    }
    HTTPFile(const char *filename) {
        pfile_ = fopen(filename, "r");
    }

    ~HTTPFile() {
        if (pfile_) fclose(pfile_);
        if (buff_) delete[] buff_;
    }

    const char *read(size_t *pread = NULL) {
        if (!pfile_) return NULL;

        if (!buff_) {
            size_t size {0};
            fseek(pfile_, 0, SEEK_END);
            size = ftell(pfile_);
            fseek(pfile_, 0, SEEK_SET);

            buff_ = new char[size];
            fread(buff_, size, 1, pfile_);

            if (pread) *pread = size;
        }
        return buff_;
    }

private:
    // Don't support
    HTTPFile& operator=(const HTTPFile&);
    HTTPFile(const HTTPFile&);

    FILE *pfile_ {NULL};
    char *buff_ {NULL};
};
