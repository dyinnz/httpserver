#include <unistd.h>     // read() / write()
#include <iostream>

#include "serve_client.h"

#include "utility.h"
#include "parse.h"
#include "file_io.h"

#include "response.h"

// debug
#include "debug_test.h"

using namespace std;

void ServeClient(int sockfd) {
    int n {0};
    char recvline[kMaxLine + 1] {0};
    char sendline[kMaxLine + 1] {0};
    
    while ( (n = read(sockfd, recvline, kMaxLine)) > 0) {
        http_log(recvline);

        Request request;
        ParseAndProcess(recvline, request);

        CreateResponseHeader(sendline, sendline+kMaxLine+1, request);

        SendResponse(sockfd, request);

        break;
    }

    //http_log("Read end.\n");
    
    //exit(-1);
}

void ParseAndProcess(const char *text, Request &request) {
    const char *p = ParseHeader(text, request);
    if (!p) return;

    GetMethod(request);
    if (Request::kPost == request.method_type) {
        size_t content_length= GetContentLength(request);
        request.entity_body.set_str(p, p+content_length);
        //http_logn(request.entity_body);
    }

    if (kSuccess != ProcessRequest(request)) return;
    return;
}

int ProcessRequest(Request &request) {
    assert(kSuccess == request.error_code);

    if (kSuccess != GetVersion(request)) {
        return request.error_code = kUnsupportVersion;
    }
    if (kSuccess != GetMethod(request)) {
        http_logn("get method error");
        return request.error_code = kUnsupportMethod;
    }
   
    switch (request.method_type) {
        case Request::kGet:
            return ProcessMethodGet(request);

        case Request::kHead:
            return ProcessMethodHead(request);

        case Request::kPost:
            return kSuccess;
        default:
            return request.error_code = kUnsupportMethod;
    }
}

int ProcessMethodGet(Request &request) {
    assert(kSuccess == request.error_code);
    strpair path;
    if (!ParseURL(request.url, path)) {
        http_logn("Parser URL error.");
        return request.error_code = kInvalidURL;
    }
    
    if (!HTTPFile(path).read(request)) {
        return request.error_code = kFileError;
    }

    return kSuccess;
}

int ProcessMethodHead(Request &request) {
    assert(kSuccess == request.error_code);
    strpair path;
    if (!ParseURL(request.url, path)) {
        http_logn("Parser URL error.");
        return request.error_code = kInvalidURL;
    }
    if (!is_exist(path)) {
        return request.error_code = kFileError;
    }
    return kSuccess;
}

int SendResponse(int sockfd, Request &request) {
    assert(request.header);

    write(sockfd, request.header, request.header_size);

    if (request.body) {
        write(sockfd, request.body, request.body_size);
    }

    return kSuccess;
}

void test_ParseURL() {
    const char cstrurl[] = "http://127.0.0.1:8000/index.html?123456#hehe";
    strpair url(cstrurl, cstrurl+strlen(cstrurl));
    strpair path;
    if (!ParseURL(url, path)) {
        cerr << "ParseURL error!" << endl;
    } else {
        cout << "ParseURL OK!" << endl;
    }
    //http_logn(path);
}

void test_Process() {
    HTTPFile file("test_text");

    const char *request_text = file.read();

    Request request;
    ParseAndProcess(request_text, request);

    char sendline[kMaxLine+1] {0};
    CreateResponseHeader(sendline, sendline+kMaxLine+1, request);
    cout << request.header << "--------" << endl;
    cout << request.body << endl;
}







