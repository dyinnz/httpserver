#include <cassert>
#include <cstring>
#include <cctype>
#include <algorithm>

#include "parse.h"
#include "serve.h"

// Interface

int ParseHeader(const char *p, Request& req) {
    assert(p);

    // Parse request-line
    p = ParseToken(p, ' ',  req.method);
    p = ParseToken(p, ' ',  req.url);
    p = ParseUntilCRLF(p, req.version);

    if (!p) {
        http_debug("Parse the request line error!\n");
        return kWrongRequestLine;
    }

    strpair key, value;
    // Parse message-header, the loop must break by return
    while (*p) {
        // Encounter CRLF, normaly return
        if ( '\r' == *p && '\n' == *(p+1) ) {
            return kSuccess;
        }
        
        // Parse the header line
        // TODO:
        p = ParseToken(p, ':', key);
        p = skip_space(p);
        p = ParseUntilCRLF(p, value);

        if (p) {
            req.keys.push_back(key);
            req.values.push_back(key);

        } else {
            http_debug("Parse the header line error!\n");
            // return kWrongHeaderLine;
            return kContinue;
        }
    }

    assert(true);       // can't reach here
    return kFailed;
}

int ExtractInformation(Request &req) {
    // TODO:
    
    if ( kSuccess != ExtractPath(req.url, req.path) ) {
        return kWrongURL;
    }

    req.method_type = (Request::MethodType)ExtractMethod(req.method);

    if ( kSuccess != ExtractVersion(req.version, &req.major_version, &req.minor_version) ) {
        return kWrongVersion;
    }

    req.request_content_length = ExtractContentLength(req);

    return kSuccess;
}

/*----------------------------------------------------------------------------*/
// Inner function

const char *skip_space(const char *p) {
    if (!p) return p; 
    while (' ' == *p) ++p;
    return p;
}

const char *ParseToken(const char *p, char tag_c, strpair &sp) {
    if (!p) return NULL;

    sp.set_beg(p);
    while (*p) {
        if (tag_c == *p) {
            sp.set_end(p);
            return p+1;
        }
        ++p;
    }
    
    return NULL;
}

const char *ParseToken(const char *p, const char *tag_s, strpair &sp) {
    if (!p) return NULL;

    sp.set_beg(p);
    while (*p) {
        if ( NULL != strchr(tag_s, *p) ) {
            sp.set_end(p); 
            return p+1;
        }
        ++p;
    }

    return NULL;
}

const char *ParseUntilCRLF(const char *p, strpair &sp) {
    if (!p) return NULL;

    sp.set_beg(p);
    while (*p) {
        if ( '\r' == *p && '\n' == *(p+1) ) {
            sp.set_end(p); 
            return p+2;
        }
        ++p;
    }

    return NULL;
}


int ExtractPath(const strpair &url, strpair &path) {
    const char *p = url.beg();
    assert(p);

    path.clear();

    if (url.case_equal_n("http://", 7)) {
        p += 7;

        if (p >= url.end()) {
            http_debug("Out of range when extracting path from url\n");
            return kWrongURL;
        }
    }

    while (p < url.end () && '/' != *p) {
        ++p;
    }
    if (url.end() == p) {
        http_debug("Out of range when extracting path from url\n");
        return  kWrongURL;
    }

    path.set_str(p, url.end());
    while (p < path.end() && *p) {

        if ('?' == *p) path.set_end(p);
        if ('#' == *p) path.set_end(p);

        ++p;
    }
    return kSuccess;
}

int ExtractMethod(const strpair &method) {
    if (method.case_equal_n("GET", 3)) {
        return Request::kGet;

    } else if (method.case_equal_n("POST", 4)) {
        return Request::kPost;

    } else if (method.case_equal_n("HEAD", 4)) {
        return Request::kHead;

    } else {
        http_debug("Undefined method!\n");
        return Request::kUndefined;
    }
}

int ExtractVersion(const strpair &version, size_t *pmajor, size_t *pminor) {
    if ( !version.case_equal_n("HTTP/", 5) ) {
        http_debug("Extract version error, no \"HTTP/\":\n");
        version.debug_print();
        return kWrongVersion;
    }

    int major {0}, minor {0};

    const char *p = version.beg() + 5;
    while ( p < version.end() && '.' != *p ) {
        if ( !isdigit(*p) ) {
            http_debug("Extract version error, is not digit:%d\n", *p);
            version.debug_print();
            return kWrongVersion;
        }
        major = major * 10 + *p++ - '0';
    }

    ++p;
    while ( p < version.end() ) {
        if ( !isdigit(*p) ) {
            http_debug("Extract version error, is not digit:%d\n", *p);
            version.debug_print();
            return kWrongVersion;
        }
        major = major * 10 + *p++ - '0';
    }

    *pmajor = major;
    *pminor = minor;

    return kSuccess;
}

// TODO: The code is ugly, rewrite sometimes
size_t ExtractContentLength(Request &req) {
    auto iter = std::find_if( req.keys.begin(), req.keys.end(),
                            [](const strpair &sp) { return sp.case_equal("Conent-Length", 14); }
                            );
    if (req.keys.end() != iter) {
        return 0;
    } else {
        return req.values[ iter - req.keys.begin() ].to_size_t();
    }
}

