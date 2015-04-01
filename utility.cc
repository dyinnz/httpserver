#include "utility.h"

/*---- const variable ----*/

const int kMaxLine = 4096;

const char kStrGet[] = "GET";

/*---- utility function ----*/

// for debug
void http_log(const strpair &sp) {
    for (const char *iter = sp.beg(); iter != sp.end(); ++iter) {
        //putchar(*iter);
        std::clog << *iter;
    }
}

void ParseText(const char *p) {
    assert(p);
    if ( NULL == (p = ParseRequestLine(p)) ) {
        return;
    }
    if ( NULL == (p = ParseHeader(p)) ) {
        return;
    }
    return;
}

const char *ParseWithTag(const char *p, char tag, strpair &sp) {
    assert(p);
    sp.set_beg(p);
    while (p && *p) {
        if (tag == *p) {
            sp.set_end(p);
            return p+1;
        }
        ++p;
    }
    return NULL;
}

const char *ParseWithTag(const char *p, const char *tags, strpair &sp) {
    assert(p);
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
    assert(p);
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

const char *ParseRequestLine(const char *p) {
    assert(p);
    strpair head, url, http_version;
    if ( NULL == (p = ParseWithTag(p, ' ', head)) ) {
        return NULL;
    }
    if ( NULL == (p = ParseWithTag(p, ' ', url)) ) {
        return NULL;
    }
    if ( NULL == (p = ParseWithCRLF(p, http_version)) ) {
        return NULL;
    }
    return p;
}

const char *ParseHeader(const char *p) {
    assert(p);
    while (p && *p) {
        if ('\r' == *p && '\n' == *(p+1)) {
            return p+2;
        }
        if (NULL == (p = ParseHeaderLine(p))) {
            return NULL;
        }
    }
    return NULL;
}

const char *ParseHeaderLine(const char *p) {
    assert(p);
    strpair key, value;
    if ( NULL == (p = ParseWithTag(p, ':', key)) ) {
        return NULL;
    }
    if (NULL == (p = skip_space(p)) ) {
        return NULL;
    }
    if ( NULL == (p = ParseWithCRLF(p, value)) ) {
        return NULL;
    }
    http_log(key);
    http_log("\n");
    http_log(value);
    http_log("\n");
    return p;
}

/*---- utility class ----*/





