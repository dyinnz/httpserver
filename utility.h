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

/*---- utility function ----*/

/*---- utility class ----*/

class strpair {
public:
    strpair() {}
    strpair(const char *beg, const char*end)
        : beg_(beg), end_(end) {}

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
        if (end_ < beg_) end_ = beg_;
    }
    
    void set_end(const char *end) {
        end_ = end;
    }

    int compare(const strpair &other) const {
        if (length() < other.length()) {
            return -1;
        } else if (length() > other.length()) {
            return 1;
        } else {
            return strncasecmp(beg_, other.beg_, length());
        }
    }

    int compare(const char *p, size_t n) const {
        if (length() < n) {
            return -1;
        } else if (length() > n) {
            return 1;
        } else {
            return strncasecmp(beg_, p, n);
        }
    }

    bool case_equal(const char *p, size_t n) const {
        if (length() !=n) {
            return false;
        } else {
            return 0 == strncasecmp(beg_, p, n);
        }
    }

    bool case_equal_n(const char *p, size_t n) const {
        if (length() < n) {
            return false;
        } else {
            return 0 == strncasecmp(beg_, p, n);
        }
    }

private:
    const char * beg_ {NULL};
    const char * end_ {NULL};
};


// log some debug information
template<class T>
inline void http_log(T value) { std::clog << value; }
template<class T>
inline void http_logn(T value) { std::clog << value << std::endl; }


inline void http_log(const strpair &sp) {
    for (const char *iter = sp.beg(); iter != sp.end(); ++iter) {
        std::clog << *iter;
    }
}
inline void http_logn(const strpair &sp) { http_log(sp); std::clog << std::endl; }

