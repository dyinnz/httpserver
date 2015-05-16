#include "configure.h"
#include "http_io.h"
#include "utility.h"

#include <unordered_map>

GlobalConfigure g_configure;

void ReadConfigure(const char *filename) {
    size_t  text_size = 0;
    char    *configure_text = ReadFileData(filename, &text_size); 

    if (!configure_text) {
        fprintf(stderr, "Read configure error!\n");
        return;
    }
    char    *p = configure_text;

    strpair key,
            temp_value;
    std::vector<strpair> values;

    printf("Reading configure...\n");

    enum    { kNewLine, kKey, kValue, kSkip, kNextLine };
    int     flag = kNewLine;
    while (*p) {

        if ('\n' == *p) {
            if (kValue == flag) {
                temp_value.set_end(p);
                values.push_back(temp_value);
            } 

            if (!values.empty()) {
                // Receiving '\n' when flag is kValue means that this is a valid configure command
                ParseConfigureCommand(key, values);

                // Debug, delete soon
                key.debug_print();
                for (auto value : values) {
                    value.debug_print();
                }
                putchar('\n');

            } else if (kNewLine != flag) {
                fprintf(stderr, "Bad configure command!\n");
            }

            key.clear();
            values.clear();
            flag = kNewLine;

        } else if (isspace(*p)) {
            if (kKey == flag) {
                key.set_end(p);
                flag = kSkip;

            } else if (kValue == flag) {
                temp_value.set_end(p);
                values.push_back(temp_value);
                flag = kSkip;
            }

        } else {
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

    free(configure_text);
}

void ParseConfigureCommand(const strpair &key, const std::vector<strpair> &values) {
    assert(values.size());
    if (key.equal_n("workers", sizeof("workers")-1)) {
        g_configure.workers = values[0].to_size_t();

    } else if (key.equal_n("max_request_header",sizeof("max_request_header")-1)) {
        g_configure.max_request_header = values[0].to_size_t();

    } else if (key.equal_n("max_response_header", sizeof("max_response_header")-1)) {
        g_configure.max_response_header = values[0].to_size_t();

    } else if (key.equal_n("log_level", sizeof("log_level")-1)) {
        g_configure.log_level = values[0].to_size_t();

    } else if (key.equal_n("log_emergency", sizeof("log_emergency")-1)) {
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

    } else {
        fprintf(stderr, "Unknown configure command!\n");

    }
};

bool InitConfigure() {
    
}



