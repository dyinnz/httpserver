#include "parse.h"
#include "utility.h"

void ParseText(const char *p, Request &request) {
    assert(p);

    if ( NULL == (p = ParseRequestLine(p, request)) ) {
        return;
    }
    if ( NULL == (p = ParseHeader(p, request)) ) {
        return;
    }
    return;
}

const char *ParseWithTag(const char *p, char tag, strpair &sp) {
    if (!p) return NULL;

    sp.set_beg(p);
    while (*p) {
        if (tag == *p) {
            sp.set_end(p);
            return p+1;
        }
        ++p;
    }
    return NULL;
}

const char *ParseWithTag(const char *p, const char *tags, strpair &sp) {
    if (!p) return NULL;

    sp.set_beg(p);
    while (p && *p) {
        if (NULL != strchr(tags, *p)) {
            sp.set_end(p); 
            return p+1;
        }
        ++p;
    }
    return NULL;
}

const char *ParseWithCRLF(const char *p, strpair &sp) {
    if (!p) return NULL;

    sp.set_beg(p);
    while (p && *p) {
        if ('\r' == *p && '\n' == *(p+1)) {
            sp.set_end(p); 
            return p+2;
        }
        ++p;
    }
    return NULL;
}

const char *ParseRequestLine(const char *p, Request &request) {
    if (!p) return NULL;

    p = ParseWithTag(p, ' ', request.method);
    p = ParseWithTag(p, ' ', request.url);
    p = ParseWithCRLF(p, request.version);

    if (!p) {
        request.state = Request::kWrongRequestLine;
    }

    return p;
}

const char *ParseHeader(const char *p, Request &request) {
    if (!p) return NULL;

    while (p && *p) {
        if ('\r' == *p && '\n' == *(p+1)) {
            return p+2;
        }
        p = ParseHeaderLine(p, request);
    }
    // p == NULL
    request.state = Request::kWrongHeader;
    return NULL;
}

const char *ParseHeaderLine(const char *p, Request &request) {
    if (!p) return NULL;

    strpair key, value;
    p = ParseWithTag(p, ':', key);
    p = skip_space(p);
    p = ParseWithCRLF(p, value);

    if (p) {
        request.keys.push_back(key);
        request.values.push_back(value);
    }

   return p;
}

Request::Error GetMethod(Request &request) {
    if (request.method.case_equal("GET", 3)) {
        request.method_type = Request::kGet;

    } else if (request.method.case_equal("POST", 4)) {
        request.method_type = Request::kPost;

    } else if (request.method.case_equal("HEAD", 4)) {
        request.method_type = Request::kHead;

    } else if (request.method.case_equal("PUT", 3)) {
        request.method_type = Request::kPut;

    } else {
        request.method_type = Request::kEmpty;
        return request.state = Request::kUndefinedMethod;
    }
    return Request::kSuccess;
}

Request::Error GetVersion(Request &request) {
    if (!request.version.case_equal_n("HTTP/", 5)) {
        http_logn("Version Error: no \"HTTP/\".");
        http_logn(request.version);
        return request.state = Request::kWrongVersion;
    }

    int major {0}, minor {0};

    const char *p = request.version.beg() + 5;
    while (p < request.version.end() && '.' != *p) {
        if (!isdigit(*p)) {
            return request.state = Request::kWrongVersion;
        }
        major *= 10;
        major += *p++ - '0';
    }

    ++p;
    while (p < request.version.end()) {
        if (!isdigit(*p)) {
            return request.state = Request::kWrongVersion;
        }
        minor *= 10;
        minor += *p++ - '0';
    }

    request.major_version = major;
    request.minor_version = minor;

    return Request::kSuccess;
}



