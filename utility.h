/*
 * Author: DY.HUST
 * Date: 2015-03-31
 * Email: ml_143@sina.com
 */

#pragma once

#include <iostream>
#include <cassert>
#include <cstring>
#include <strings.h>

#define DEBUG

// previous declaration
class strpair;

/*---- const variable ----*/

extern const int kMaxLine;
extern const char kStrGet[];

/*---- utility function ----*/

// log some debug information
inline void http_log(const char *p) { std::clog << p; }

void http_log(const strpair &sp);

// string
inline const char *skip_space(const char *p) {
    if (p && ' ' == *p) ++p;
    return p;
}

// Parse the http message
void ParseText(const char *p);

const char *ParseWithTag(const char *p, char tag, strpair &sp);
const char *ParseWithTag(const char *p, const char *tags, strpair &sp);
const char *ParseWithCRLF(const char *p, strpair &sp);

const char *ParseRequestLine(const char *p);
const char *ParseHeader(const char *p);
const char *ParseHeaderLine(const char *p);

/*---- utility class ----*/

class strpair {
public:
    bool empty() const {
        return beg_ == end_;
    }

    size_t length() const {
        assert(end_ - beg_);
        return end_ - beg_;
    }

    const char *beg() const { return beg_; }
    const char *end() const { return end_; }

    void clear() { 
        beg_ = NULL; end_ = NULL; 
    }

    void set_str(const char *beg, const char *end) {
        beg_ = beg; end_ = end;
    }

    void set_beg(const char *beg) {
        beg_ = beg;
    }
    
    void set_end(const char *end) {
        end_ = end;
    }

    int compare(const strpair &other) const {
        if (length() < other.length()) {
            return -1;
        } else if (length() > other.length()) {
            return 1;
        }
        return strncasecmp(beg_, other.beg_, length());
    }

private:
    const char * beg_ {NULL};
    const char * end_ {NULL};
};


