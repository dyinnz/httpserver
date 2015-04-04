#include "response.h"
#include "parse.h"

const char kStatus200[] = " 200 OK";
const int kStatus200Len = 7;

const char kStatus400[] = " 400 Bad Request";
const int kStatus400Len = 16;

/*----------------------------------------------------------------------------*/

char *CreateResponseHeader(char *beg, const char *end, Response &response) {
    response.header = beg; 

    char *p { beg };
    p = AppendStatusLine(p, 200);
    if (response.body) {
        p = AppendEntityHeader(p, response.data_size);
    }
    p = AppendCRLF(p);
    
    response.header_size = p - beg;
    return beg;
}

char *AppendStatusLine(char *p, int status) {
    p = copy_move(p, "HTTP/1.1", 8);
    
    switch (status) {
        case 200:
            p = copy_move(p, kStatus200, kStatus200Len);
            break;

        default:
            p = copy_move(p, kStatus400, kStatus400Len);
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

