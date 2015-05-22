#include "configure.h"
#include "http_io.h"
#include "utility.h"
#include "memory_pool.h"

#include <unordered_map>

/******************************************************************************/
/* Global variable */

GlobalConfigure g_configure;
static const int kPoolDefaultSize = 4096;

/******************************************************************************/

bool InitConfigure() {
    if (!LoadDefaultConfigure()) return false;

    MemoryPool pool(kPoolDefaultSize);
    ReadConfigure("configure", pool);

    ReleaseConfigureResource();

    return true;
}

/******************************************************************************/
/* Read text from configure file and split it into keys and values */

void ReadConfigure(const char *filename, MemoryPool &pool) {

    // Read text from text
    size_t  text_size = 0;
    if (! (g_configure.configure_text = ReadFileData(filename, &text_size, pool)) ) {
        fprintf(stderr, "Read configure error!\n");
        return;
    }
    const char *p = g_configure.configure_text;

    // Variable storing tokens
    strpair key,
            temp_value;
    std::vector<strpair> values;

    printf("Reading configure...\n");

    // A simple state machine
    enum    { kNewLine, kKey, kValue, kSkip, kNextLine };
    int     flag = kNewLine;
    while (*p) {

        if ('\n' == *p) {
            // Push the last value token
            if (kValue == flag) {
                temp_value.set_end(p);
                values.push_back(temp_value);
            } 

            if (!values.empty()) {
                // Receiving '\n' when flag is kValue means that this is a valid configure command
                ParseConfigureCommand(key, values);

            } else if (kNewLine != flag) {
                fprintf(stderr, "Bad configure command!\n");
            }

            // Clear the tokens and reset flag to receive a new line.
            key.clear();
            values.clear();
            flag = kNewLine;

        } else if (isspace(*p)) {

            // Receiving space means the end of a token
            if (kKey == flag) {
                key.set_end(p);
                flag = kSkip;

            } else if (kValue == flag) {
                temp_value.set_end(p);
                values.push_back(temp_value);
                flag = kSkip;
            }

        } else {
            // Receiving other char means the begin of a token
            if (kSkip == flag) {
                temp_value.set_beg(p);
                flag = kValue; 

            } else if (kNewLine == flag) {
                key.set_beg(p);
                flag = kKey;
            }
        }

        ++p;
    }
}

/******************************************************************************/
/* Dirty code... But it's easy to implement. */

bool ParseConfigureCommand(const strpair &key, const std::vector<strpair> &values) {
    assert(values.size());

    // The number of values must be one, and the one must be a number, but
    // haven't do any check for convert. So the number may be unknown when
    // passing non-digit char.
    if (key.equal_n("workers", sizeof("workers")-1)) {
        g_configure.workers = values[0].to_size_t();

    } else if (key.equal_n("max_request_header",sizeof("max_request_header")-1)) {
        g_configure.max_request_header = values[0].to_size_t();

    } else if (key.equal_n("max_response_header", sizeof("max_response_header")-1)) {
        g_configure.max_response_header = values[0].to_size_t();

    } else if (key.equal_n("log_level", sizeof("log_level")-1)) {
        g_configure.log_level = values[0].to_size_t();

    } else if (key.equal_n("memory_pool_size", sizeof("memory_pool_size")-1)) {
        g_configure.memory_pool_size = values[0].to_size_t();
    }

    // The values are string, and the number may be no more than one. Because
    // some configure needs.
    else if (key.equal_n("log_emergency", sizeof("log_emergency")-1)) {
        LoadLogFile(&g_configure.log_fp[kEmergency], values[0]);

    } else if (key.equal_n("log_alert", sizeof("log_alert")-1)) {
        LoadLogFile(&g_configure.log_fp[kAlert], values[0]);

    } else if (key.equal_n("log_critical", sizeof("log_critical")-1)) {
        LoadLogFile(&g_configure.log_fp[kCritical], values[0]);

    } else if (key.equal_n("log_error", sizeof("log_error")-1)) {
        LoadLogFile(&g_configure.log_fp[kError], values[0]);

    } else if (key.equal_n("log_warning", sizeof("log_warning")-1)) {
        LoadLogFile(&g_configure.log_fp[kWarning], values[0]);

    } else if (key.equal_n("log_notice", sizeof("log_notice")-1)) {
        LoadLogFile(&g_configure.log_fp[kNotice], values[0]);

    } else if (key.equal_n("log_inform", sizeof("log_inform")-1)) {
        LoadLogFile(&g_configure.log_fp[kInform], values[0]);

    } else if (key.equal_n("log_debug", sizeof("log_debug")-1)) {
        LoadLogFile(&g_configure.log_fp[kDebug], values[0]);

    } else if (key.equal_n("location", sizeof("location")-1)) {
        // TODO

    }

    // Have not the word, so the user must enters wrong configure word
    else {
        fprintf(stderr, "Unknown configure command!\n");
        return false;
    }

    return true;
}


/******************************************************************************/
/* If haven't the configuraion file, program will use default configuration */

bool LoadDefaultConfigure() {
    g_configure.workers             = 4;        
    g_configure.max_request_header  = 4096;     // 4k
    g_configure.max_response_header = 4096;
    g_configure.log_level           = kNotice;
    g_configure.memory_pool_size    = 4096;

    /*
    FILE *null_fp = fopen("/dev/null", "w");
    if (!null_fp) {
        printf("Open /dev/null error!\n");
        return false;
    }
    */

    g_configure.log_fp[kEmergency]  = stderr;
    g_configure.log_fp[kAlert]      = stderr;
    g_configure.log_fp[kCritical]   = stderr;
    g_configure.log_fp[kError]      = stderr;
    g_configure.log_fp[kWarning]    = stdout;
    g_configure.log_fp[kNotice]     = stdout;
    g_configure.log_fp[kInform]     = stdout;
    g_configure.log_fp[kDebug]      = stdout;

    return true;
}

/******************************************************************************/
/* Release memory and file point  */

void ReleaseConfigureResource() {
    // 
}

/******************************************************************************/

void LoadLogFile(FILE **ppf, const strpair &filename) {
    char temp = *filename.end();
    *(char*)filename.end() = '\0';

    *ppf = fopen(filename.beg(), "a");

    *(char*)filename.end() = temp;

    if (NULL == *ppf) {
        fprintf(stderr, "Open log file error!");
    }
}

/******************************************************************************/
