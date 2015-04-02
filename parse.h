#pragma once

#include "utility.h"
#include <vector>

/*----------------------------------------------------------------------------*/

class Request {
public:
    enum Error {
        kSuccess, kFailed, kWrongRequestLine, kWrongHeader,
        kWrongVersion, kUndefinedMethod,
    };

    Error state { kSuccess };

    strpair method,
            url, 
            version;

    std::vector<strpair> keys;
    std::vector<strpair> values; 

    strpair entity_body;
    
    enum MethodType {
        kEmpty, kUndefined,
        kGet, kPost, kPut, kHead,
    };

    MethodType method_type {kEmpty};

    size_t major_version {0}, minor_version {0};
};

// string
inline const char *skip_space(const char *p) {
    if (p && ' ' == *p) ++p;
    return p;
}

// Parse the http message
void ParseText(const char *p, Request &request);

const char *ParseWithTag(const char *p, char tag, strpair &sp);
const char *ParseWithTag(const char *p, const char *tags, strpair &sp);
const char *ParseWithCRLF(const char *p, strpair &sp);

const char *ParseRequestLine(const char *p, Request &request);
const char *ParseHeader(const char *p, Request &request);
const char *ParseHeaderLine(const char *p, Request &request);

Request::Error GetMethod(Request &request);
Request::Error GetVersion(Request &request);
