#include "configure.h"
#include "http_io.h"
#include "utility.h"

#include <unordered_map>

/******************************************************************************/
/* Global variable */

GlobalConfigure g_configure;

/******************************************************************************/

bool InitConfigure() {
    if (!LoadDefaultConfigure()) return false;

    ReadConfigure("configure");

    if (!LoadUserConfigure()) return false;

    return true;
}

/******************************************************************************/
/* Read text from configure file and split it into keys and values */

void ReadConfigure(const char *filename) {

    // Read text from text
    size_t  text_size = 0;
    if (! (g_configure.configure_text = ReadFileData(filename, &text_size)) ) {
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

void ParseConfigureCommand(const strpair &key, const std::vector<strpair> &values) {
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

    }

    // The values are string, and the number may be no more than one. Because
    // some configure needs.
    else if (key.equal_n("log_emergency", sizeof("log_emergency")-1)) {
        g_configure.log_filename[kEmergency] = values[0];

    } else if (key.equal_n("log_alert", sizeof("log_alert")-1)) {
        g_configure.log_filename[kAlert] = values[0];

    } else if (key.equal_n("log_critical", sizeof("log_critical")-1)) {
        g_configure.log_filename[kCritical] = values[0];

    } else if (key.equal_n("log_error", sizeof("log_error")-1)) {
        g_configure.log_filename[kError] = values[0];

    } else if (key.equal_n("log_warning", sizeof("log_warning")-1)) {
        g_configure.log_filename[kWarning] = values[0];

    } else if (key.equal_n("log_notice", sizeof("log_notice")-1)) {
        g_configure.log_filename[kNotice] = values[0];

    } else if (key.equal_n("log_inform", sizeof("log_inform")-1)) {
        g_configure.log_filename[kInform] = values[0];

    } else if (key.equal_n("log_debug", sizeof("log_debug")-1)) {
        g_configure.log_filename[kDebug] = values[0];

    } else if (key.equal_n("location", sizeof("location")-1)) {
        // TODO

    }

    // Have not the word, so the user must enters wrong configure word
    else {
        fprintf(stderr, "Unknown configure command!\n");
    }
};


/******************************************************************************/
/* If haven't the configuraion file, program will use default configuration */

bool LoadDefaultConfigure() {
    g_configure.workers             = 4;        
    g_configure.max_request_header  = 4096;     // 4k
    g_configure.max_response_header = 4096;
    g_configure.log_level           = kNotice;

    // Othe part of the program won't use the variable, just empty it
    memset(g_configure.log_filename, 0, sizeof(g_configure.log_filename));

    FILE *null_fp = fopen("/dev/null", "w");
    if (!null_fp) {
        printf("Open /dev/null error!\n");
        return false;
    }

    g_configure.log_fp[kEmergency]  = stderr;
    g_configure.log_fp[kAlert]      = stderr;
    g_configure.log_fp[kCritical]   = stderr;
    g_configure.log_fp[kError]      = stderr;
    g_configure.log_fp[kWarning]    = stdout;
    g_configure.log_fp[kNotice]     = stdout;
    g_configure.log_fp[kInform]     = null_fp;
    g_configure.log_fp[kDebug]      = null_fp;

    return true;
}

/******************************************************************************/
/* Some configuration won't be effective at once, which need further handle */

bool LoadUserConfigure() {

    fclose(g_configure.log_fp[kInform]);

    // load log file
    for (int i = 0; i < 8; ++i) if (!g_configure.log_filename[i].empty()) {

        char temp = *g_configure.log_filename[i].end();
        *(char*)g_configure.log_filename[i].end() = '\0';

        FILE *fp = fopen(g_configure.log_filename[i].beg(), "a");
        if (NULL == fp) {
            fprintf(stderr, "Open log file error!");
            return false;

        } else {
            g_configure.log_fp[i] = fp;
        }

        *(char*)g_configure.log_filename[i].end() = temp;
    }

    return true;
}

/******************************************************************************/
/* Release memory and file point  */

void ReleaseConfigureResource() {

    for (int i = 0; i < 8; ++i) {
        if (stderr != g_configure.log_fp[i] && stdout != g_configure.log_fp[i]) {
            fclose(g_configure.log_fp[i]); 
        }
    }
    free((void*)g_configure.configure_text);
}

/******************************************************************************/

