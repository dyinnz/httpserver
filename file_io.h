#pragma once

#include <cstdio>

bool is_exist(const char *filename);

class HTTPFile {
public:
    HTTPFile(const char *filename) {
        pfile_ = fopen(filename);
    }

    ~HTTPFile() {
        if (pfile) fclose(pfile);
    }

    const char *Read(const char *filename=NULL, size_t &) {
    }

private:
    // Don't support
    HTTPFile& operator=(const HTTPFile&);
    HTTPFile(const HTTPFile&);

    FILE *pfile_ {NULL};
    char *buff_ {NULL};
};
