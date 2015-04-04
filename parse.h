#pragma once

#include "utility.h"
#include <vector>

/*----------------------------------------------------------------------------*/

class Request {
public:
    int                     error_code {0};
    int                     status {0};

    size_t                  major_version {0},
                            minor_version {0};

    size_t                  body_size {0};
    size_t                  header_size {0};

    enum MethodType {
        kEmpty,
        kUndefined,
        kGet,
        kPost,
        kHead,
    };

    MethodType              method_type {kEmpty};

    char                    *header {NULL};
    char                    *body {NULL};

    strpair                 method,
                            url, 
                            version;

    strpair                 entity_body;

    std::vector<strpair>    keys;
    std::vector<strpair>    values; 

    ~Request() {
        if (body) delete [] body;
    }
};

// string
inline const char *skip_space(const char *p) {
    if (p && ' ' == *p) ++p;
    return p;
}

// Parse the http message

const char *ParseWithTag(const char *p, char tag, strpair &sp);
const char *ParseWithTag(const char *p, const char *tags, strpair &sp);
const char *ParseWithCRLF(const char *p, strpair &sp);

const char *ParseHeader(const char *p, Request &request);
const char *ParseHeaderLine(const char *p, Request &request);

// TODO:
//const char *ParseFieldValue(const char *p, Request &request);

// second parse
bool ParseURL(const strpair &sp, strpair &out_url);
const char *ParseHostname(const char *p, const char *end);

int GetMethod(Request &request);
int GetVersion(Request &request);
size_t GetContentLength(Request &request);

