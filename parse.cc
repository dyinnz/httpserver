#include "parse.h"
#include "utility.h"

#include <algorithm>

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

const char *ParseHeader(const char *p, Request &request) {
    assert(p);

    // Parse request-line
    p = ParseWithTag(p, ' ', request.method);
    p = ParseWithTag(p, ' ', request.url);
    p = ParseWithCRLF(p, request.version);

    if (!p) {
        request.error_code = kWrongRequestLine;
        return NULL;
    }

    // Parse message-header, the loop must break by return
    while (*p) {
        // Encounter CRLF, normaly return
        if ('\r' == *p && '\n' == *(p+1)) {
            return p+2;
        }

        // Unexpected text, return NULL
        if ( NULL == (p = ParseHeaderLine(p, request)) ) {
            request.error_code = kWrongHeader;
            return NULL;
        }
    }

    // Skip the CRLF line
    if ('\r' == *p && '\n' == *(p+1)) {
        p += 2;
        return p;
    } else {
        return NULL;
    }
}

const char *ParseHeaderLine(const char *p, Request &request) {
    assert(p);

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

bool ParseURL(const strpair &sp, strpair &out_url) {
    const char *p = sp.beg();
    assert(p);

    out_url.empty();

    if (sp.case_equal_n("http://", 7)) {
        if (p + 7 >= sp.end()) {
            //cerr << "out of range." << endl;
            return false;
        }

        if ( NULL == (p = ParseHostname(p + 7, sp.end())) ) {
            //cerr << "host name!" << endl;
            return false;
        }
    }
    
    if ('/' != *p) {
        //cerr << "no root path." << endl;
        return false;
    }

    out_url.set_str(p, sp.end());
    while (p < sp.end() && *p) {
        if ('?' == *p) {
            out_url.set_end(p);
        }
        if ('#' == *p && !out_url.end()) {
            out_url.set_end(p);
        }
        ++p;
    }
    return true;
}

const char* ParseHostname(const char *p, const char *end) {
    assert(p);
    size_t dot_cnt {0};
    while (p < end && '/' != *p) {
        if ('.' == *p) {
            ++dot_cnt;
        } else if (':' == *p) {
            // nothing
        } else if (!isdigit(*p)) {
            return NULL;
        }
        ++p;
    }
    if (3 != dot_cnt) {
        return NULL;
    }
    return p;
}


int GetMethod(Request &request) {
    http_logn(__func__);
    http_logn(request.method);
    if (request.method.case_equal_n("GET", 3)) {
        request.method_type = Request::kGet;

    } else if (request.method.case_equal_n("POST", 4)) {
        request.method_type = Request::kPost;

    } else if (request.method.case_equal_n("HEAD", 4)) {
        request.method_type = Request::kHead;

    } else {
        request.method_type = Request::kUndefined;
        return request.error_code = kUndefinedMethod;
    }

    return kSuccess;
}

int GetVersion(Request &request) {
    if (!request.version.case_equal_n("HTTP/", 5)) {
        http_logn("Version Error: no \"HTTP/\".");
        http_logn(request.version);
        return request.error_code = kWrongVersion;
    }

    int major {0}, minor {0};

    const char *p = request.version.beg() + 5;
    while (p < request.version.end() && '.' != *p) {
        if (!isdigit(*p)) {
            return request.error_code = kWrongVersion;
        }
        major *= 10;
        major += *p++ - '0';
    }

    ++p;
    while (p < request.version.end()) {
        if (!isdigit(*p)) {
            return request.error_code = kWrongVersion;
        }
        minor *= 10;
        minor += *p++ - '0';
    }

    request.major_version = major;
    request.minor_version = minor;

    return kSuccess;
}

size_t GetContentLength(Request &request) {
    auto f = [](const strpair &sp) {
        return sp.case_equal("Content-Length", 14 );
    };
    auto iter = std::find_if(request.keys.begin(), request.keys.end(), f);

    if (request.keys.end() == iter) return 0;
    size_t index = iter - request.keys.begin();
    
    return request.values[index].to_size_t();
}



