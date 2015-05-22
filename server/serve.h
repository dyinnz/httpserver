/*
 * Author:  DY.HUST
 * Date:    2015-04-15
 * Email:   ml_143@sina.com
 */

#pragma once

#include <cstring>
#include <vector>

#include "utility.h"

class MemoryPool;

class Request {
public:
    int                     error_code {0};
    //int                     status {0};
    size_t                  major_version {0};
    size_t                  minor_version {0};

    size_t                  request_content_length {0};

    size_t                  response_header_length {0};
    size_t                  response_content_length {0};

    enum MethodType {
        kUndefined = 0,
        kGet,
        kPost,
        kHead
    };
    MethodType              method_type {kUndefined};

    char                    *header {NULL};
    char                    *body {NULL};

    strpair                 method,
                            url,
                            version,
                            path,
                            
                            entity_body;

    std::vector<strpair>    keys;
    std::vector<strpair>    values;
};

// Serve client with the sock, read request and process it, and then send
// response to the client
void ServeClient(int sockfd);

// Return status code
int ProcessRequest(Request &req, MemoryPool &pool);

void CreateErrorResponse(Request &req);

// inner 
void debug_ServeClient(int sockfd);

inline char *copy_move(char * p, const char *str, size_t len) {
    strncpy(p, str, len);
    return p + len;
}

