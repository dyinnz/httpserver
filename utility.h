/*
 * Author: DY.HUST
 * Date: 2015-03-31
 * Email: ml_143@sina.com
 */

#pragma once

#include <cstdio>
#include <cstdarg>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <strings.h>

#define DEBUG

static const int kMaxHeader = 8096;
static const int kMaxWorkProcess = 10;

extern FILE *g_log_fp[8];

// Status code
enum HTTPError {
    kSuccess = 0,
    kContinue = 0,
    kFailed,

    // parse error
    kWrongRequestLine,
    kWrongHeaderLine,
    kWrongVersion,
    kWrongURL,
    kUndefinedMethod,

    // process error
    kUnsupportVersion,
    kUnsupportMethod,
    kInvalidURL,
    kFileError,
};

enum LogLevel {
    kEmergency,
    kAlert,
    kCritical,
    kError,
    kWarning,
    kNotice,
    kInform,
    kDebug,
};

/*----------------------------------------------------------------------------*/

// POD class 
class strpair {
public:
    strpair(const char *beg = NULL, const char *end = NULL) : beg_(beg), end_(end) {}

    bool empty() const {
        return beg_ == end_;
    }

    size_t length() const {
        assert(end_ - beg_ >= 0);
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
        assert(end_ >= beg_);
        end_ = end;
    }

    bool equal_n(const char *p, size_t n) const {
        return 0 == strncmp(beg_, p, n);
    }

    bool case_equal_n(const char *p, size_t n) const {
        if (length() != n) {
            return false;
        } else {
            return 0 == strncasecmp(beg_, p, n);
        }
    }

    bool case_equal_firstn(const char *p, size_t n) const {
        if (length() < n) {
            return false;
        } else {
            return 0 == strncasecmp(beg_, p, n);
        }
    }

    size_t to_size_t() const {
        size_t num {0};
        for (const char *p = beg_; p < end_; ++p) {
            //putchar(*p);
            num = num*10 + (*p - '0');
        }
        return num;
    }

    void debug_print() const {
#ifdef DEBUG
        for (const char *p = beg_; p < end_; ++p) {
            putchar(*p);
        }
        putchar('\n');
#endif
    }

private:
    const char * beg_;
    const char * end_;
};

// Output

inline void http_error(const char *formart, ...) {
    va_list va;
    va_start(va, formart);
    vfprintf(stderr, formart, va);
    va_end(va);
} 

// TODO: should set stream
inline void http_log(const char *formart, ...) {
    va_list va; 
    va_start(va, formart);
    vfprintf(stdout, formart, va);
    va_end(va);
}

inline void http_log(int level, const char *formart, ...) {
    assert(0 <= level && level < 8);
    va_list va;
    va_start(va, formart);
    vfprintf(g_log_fp[level], formart, va);
    va_end(va);
}

#ifdef DEBUG

inline void http_debug(const char *formart, ...) {
    va_list va;
    va_start(va, formart);
    vprintf(formart, va);
    va_end(va);
}

#else

inline void http_debug(const char *formart, ...) {}

#endif

// Memory

inline void* http_alloc(size_t size) {
    return malloc(size);
}

inline void http_free(void *p) {
    free(p);
}

