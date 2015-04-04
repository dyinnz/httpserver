#include "response.h"
#include "parse.h"

const char kStatus200[] = " 200 OK";
const int kStatus200Len = 7;

const char kStatus400[] = " 400 Bad Request";
const int kStatus400Len = 16;

const char kStatus201[] = " 201 Created";
const char kStatus404[] = " 404 Not Found";
const char kStatus501[] = " 501 Not Implemented";
const char kStatus505[] = " 505 Unsupported Version";

const int kStatusCreated        = 201;
const int kStatusOk             = 200;
const int kStatusBadRequest     = 400;
const int kStatusNotFound       = 404;
const int kStatusNotAllowed     = 405;
const int kStatusServerError    = 500;
const int kStatusNotImplemented = 501;
const int kStatusVersion        = 505;

/*----------------------------------------------------------------------------*/

char *CreateResponseHeader(char *beg, const char *end, Request &request) {
    request.header = beg; 

    char *p { beg };
    p = AppendStatusLine(p, request);
    if (request.body) {
        p = AppendEntityHeader(p, request.body_size);
    }
    if (Request::kPost == request.method_type) {
        p = AppendEntityHeader(p, 0);
    }
    p = AppendCRLF(p);
    
    request.header_size = p - beg;
    return beg;
}

char *AppendStatusLine(char *p, Request &request) {
    p = copy_move(p, "HTTP/1.1", 8);

    // check the status code
    int status_code {0};
    switch (request.error_code) {
        case kSuccess: 
            switch (request.method_type) {
                case Request::kGet:
                case Request::kHead:
                    status_code = kStatusOk;
                    p = copy_move(p, kStatus200, kStatus200Len);
                    break;
                case Request::kPost:
                    status_code = kStatusCreated;
                    p = copy_move(p, kStatus201, strlen(kStatus201));
                    break;
                default:
                    assert(false);

            }
            break;

        case kFailed: 
        case kWrongRequestLine:
        case kWrongVersion:
        case kUndefinedMethod:
        case kInvalidURL:
            status_code = kStatusBadRequest;
            p = copy_move(p, kStatus400, kStatus400Len);
            break;

        case kFileError:
            status_code = kStatusNotFound;
            p = copy_move(p, kStatus404, strlen(kStatus404));
            break;

        case kUnsupportMethod:
            status_code = kStatusNotImplemented;
            p = copy_move(p, kStatus501, strlen(kStatus501));
            break;

        case kUnsupportVersion:
            status_code = kStatusVersion;
            p = copy_move(p, kStatus505, strlen(kStatus505));
            break;

        default:
            assert(false);
    }

    p = AppendCRLF(p);

    return p;
}

char *AppendHeader(char *p, const char *keys, const char *values) {
    p = copy_move(p, keys, strlen(keys)); 
    *p++ = ':';
    *p++ = ' ';
    p = copy_move(p, values, strlen(values));
    p = AppendCRLF(p);
    
    return p;
}

char *AppendEntity(char *p, const char *data, size_t size) {
    p = AppendHeader(p, "Content-Type", "text/html; charset=UTF-8");

    p = copy_move(p, "Content-Length: ");
    p += snprintf(p, 16, "%d\r\n", size);

    p = AppendCRLF(p);

    memcpy(p, data, size);

    //p = AppendCRLF(p + size);

    return p;
}

char *AppendEntityHeader(char *p, size_t data_size) {
    //assert(data_size);
    p = AppendHeader(p, "Content-Type", "text/html; charset=UTF-8");

    p = copy_move(p, "Content-Length: ");
    p += snprintf(p, 16, "%d\r\n", data_size);
    return p;
}

