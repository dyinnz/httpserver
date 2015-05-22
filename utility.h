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

/******************************************************************************/
/* Constant */

static const char *kLogTag[8] = {
    "Emergency: ",
    "Alert: ",
    "Critical: ",
    "Error: ",
    "Warning: ",
    "Notice: ",
    "Inform: ",
    "Debug: ",
};

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

/******************************************************************************/
/* Structure stores the begin and end of a string */

class strpair {
public:
    // strpair(const char *beg = NULL, const char *end = NULL) : beg_(beg), end_(end) {}

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

/******************************************************************************/
/* Configure struct and declare a extern global variable containing all the 
 * configure variables
 */

struct GlobalConfigure {
    size_t      workers,                    
                max_request_header,
                max_response_header,
                log_level,
                memory_pool_size;

    FILE        *log_fp[8];
    
    const char  *configure_text;
};

// Do not change the g_configure after call InitConfigure()
extern GlobalConfigure g_configure;

/******************************************************************************/
/* Log and debug output */

inline void http_log(size_t level, const char *formart, ...) {
    assert(level < 8);
    if (level <= g_configure.log_level) {
        va_list va;
        va_start(va, formart);
        fprintf(g_configure.log_fp[level], kLogTag[level]);
        vfprintf(g_configure.log_fp[level], formart, va);
        va_end(va);
    }
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

/******************************************************************************/
/* Memory allocation */

inline void* http_alloc(size_t size) {
    return malloc(size);
}

inline void http_free(void *p) {
    free(p);
}

/* End */
/******************************************************************************/
