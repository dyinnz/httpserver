#include "serve.h"
#include "http_io.h"
#include "parse.h"

#include <iostream>
using namespace std;

static const char kResponse200[] = "HTTP/1.1 200 OK\r\n";
static const char kResponse400[] = "HTTP/1.1 400 Bad Request\r\n";
static const char kResponse404[] = "HTTP/1.1 404 Not Found\r\n";
static const char kResponse500[] = "HTTP/1.1 500 Internal Server Error\r\n";
static const char kResponse501[] = "HTTP/1.1 501 Not Implemented\r\n";
static const char kCRLF[] = "\r\n";
static const char kConnectClose[] = "Connection: close\r\n";

/*----------------------------------------------------------------------------*/

void ServeClient(int sockfd) {
    char request_header[kMaxHeader+1] {0};
    char response_header[kMaxHeader+1] {0};
    char *request_body {NULL};
    size_t readn {0};

    while (true) {
        if (0 == (readn = ReadRequestHeader(sockfd, request_header, kMaxHeader+1)) ) {
            http_debug("Read empty header from sock!\n");
            break;
        }

        http_debug("Receive request header >>>>\n%s<<<<\n",
                   request_header);

        Request req;
        if ( kSuccess != ParseHeader(request_header, req) ) {
            http_debug("Parse the header error!\n"); 
            break;
        }

        if ( kSuccess != ExtractInformation(req) ) {
            http_debug("Extract information from request error!\n");
        }

        if ( Request::kPost == req.method_type && 0 != req.request_content_length ) {

            // TODO: There is a bug, part or all of the body has be read in the request header buff
            /*
            request_body = static_cast<char*>( http_alloc(req.request_content_length+1) );

            if (req.request_content_length != ReadRequestBody(sockfd, 
                                                      request_body, 
                                                      req.request_content_length) ) {
                http_debug("Read body from sock error!\n");

            } else {
                http_debug("Receive request body >>>>\n%s<<<<\n",
                           request_body);
                req.entity_body.set_str(request_body, 
                                        request_body+req.request_content_length);
            }
            */
        }

        req.header = response_header;
        ProcessRequest(req);

        if (kSuccess != req.error_code) {
            http_debug("Create response header to bad request!\n");
            CreateErrorResponse(req);
        }

        http_debug("Response Header >>>>\n%s<<<<<\n", req.header);
        SendSocketData(sockfd, req.header, req.response_header_length);
        if (req.body) {
            http_debug("Begin send body:\naddress: %x length: %d\n", req.body, req.response_content_length);
            SendSocketData(sockfd, req.body, req.response_content_length);
        }
        
        if (!request_body) {
            http_free(request_body);
            request_body = NULL;
        }
        break;
    }

    if (!request_body) http_free(request_body);
}

int ProcessRequest(Request &req) { 
    switch (req.method_type) {
        case Request::kGet:
            req.body = ReadFileData(req.path, &req.response_content_length);
            if (!req.body) {
                http_debug("Read data from file error!\n");
                req.path.debug_print();
                req.error_code = kFileError;
                return kContinue;
            }
            http_debug("File Data >>>>\n%s<<<<\n", req.body);
            break;

        case Request::kHead:
            if ( IsFileExist(req.path) ) {
                http_debug("Access file error!\n");
                req.error_code = kFileError;
                return kContinue;
            }
            break;

        case Request::kPost:
            http_debug("Ingore method \"POST\"\n");
            // TODO:
            break;

        default:
            req.error_code = kUnsupportMethod;
            return kContinue;
    }

    if (kSuccess == req.error_code) {
        char *p = req.header;

        p = copy_move(p, kResponse200, sizeof(kResponse200)-1);
        p = copy_move(p, kConnectClose, sizeof(kConnectClose)-1);

        if (0 != req.response_content_length) {
            char temp_buff[30] {0};

            snprintf(temp_buff, 30, "Content-Length: %d\r\n",
                     req.response_content_length);
            p = copy_move(p, temp_buff, strlen(temp_buff));
        }

        p = copy_move(p, kCRLF, sizeof(kCRLF)-1);

        req.response_header_length = p - req.header;
        //http_debug("true length: %d\ncalc length: %d\n", strlen(req.header), req.response_header_length);
        assert( strlen(req.header) == req.response_header_length );
    }

    return kSuccess;
}

void CreateErrorResponse(Request &req) {
    char *p = req.header;

    switch (req.error_code) {
        case kFailed:
            p = copy_move(p, kResponse500, sizeof(kResponse500)-1);
            break;

        case kWrongRequestLine:
        case kWrongVersion:
        case kUndefinedMethod:
            p = copy_move(p, kResponse400, sizeof(kResponse400)-1);
            break;

        case kUnsupportMethod:
            p = copy_move(p, kResponse501, sizeof(kResponse501)-1);
            break;

        default:
            p = copy_move(p, kResponse500, sizeof(kResponse500)-1); 
            break;
    }

    p = copy_move(p, kConnectClose, sizeof(kConnectClose)-1);
    p = copy_move(p, kCRLF, sizeof(kCRLF)-1);
    req.response_header_length = p - req.header;
}

void debug_ServeClient(int sockfd) {
    char request_header[kMaxHeader+1] = {
        "GET /index.html HTTP/1.1\r\n"
        "User-Agent: curl/7.35.0\r\n"
        "Host: 127.0.0.1:8000\r\n"
        "Accept: */*\r\n"
        "Content-Length: 10\r\n\r\n"
        "1234567890"
    };
    char response_header[kMaxHeader+1] {0};
    char *request_body {NULL};
    size_t readn {0};

    while (true) {
        http_debug("Receive request header >>>>\n%s<<<<\n",
                   request_header);

        Request req;
        if ( kSuccess != ParseHeader(request_header, req) ) {
            http_debug("Parse the header error!\n"); 
            break;
        }

        if ( kSuccess != ExtractInformation(req) ) {
            http_debug("Extract information from request error!\n");
        }

        if ( Request::kPost == req.method_type && 0 != req.request_content_length ) {

            // TODO: There is a bug, part or all of the body has be read in the request header buff
            /*
            // request_body = new char[req.request_content_length+1];
            request_body = static_cast<char*>( http_alloc(req.request_content_length+1) );

            if (req.request_content_length != ReadRequestBody(sockfd, 
                                                      request_body, 
                                                      req.request_content_length) ) {
                http_debug("Read body from sock error!\n");

            } else {
                http_debug("Receive request body >>>>\n%s<<<<\n",
                           request_body);
                req.entity_body.set_str(request_body, 
                                        request_body+req.request_content_length);
            }
            */
        }

        req.header = response_header;
        ProcessRequest(req);

        if (kSuccess != req.error_code) {
            http_debug("Create response header to bad request!\n");
            CreateErrorResponse(req);
        }

        http_debug("Response Header >>>>\n%s<<<<<\n", req.header);
//        SendSocketData(sockfd, req.header, req.response_header_length);
        if (req.body) {
            http_debug("Begin send body:\naddress: %x length: %d\n", req.body, req.response_content_length);
//            SendSocketData(sockfd, req.body, req.response_content_length);
        }
        
        if (!request_body) {
            http_free(request_body);
            request_body = NULL;
        }
        break;
    }

    if (!request_body) http_free(request_body);
}

