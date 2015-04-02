#include "utility.h"

/*---- const variable ----*/

const int kMaxLine = 4096;

/*---- utility function ----*/

// for debug
void http_log(const strpair &sp) {
    for (const char *iter = sp.beg(); iter != sp.end(); ++iter) {
        //putchar(*iter);
        std::clog << *iter;
    }
}

/*---- utility class ----*/





