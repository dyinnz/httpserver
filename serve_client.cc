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
        ParseHeader(recvline, request);

        //size_t size = test_CreateResponse(sendline, sendline + kMaxLine, request);
        //write(sockfd, sendline, size);

        break;
    }

    //http_log("Read end.\n");
    
    //exit(-1);
}

int ProcessRequest(Request &request, Response &response) {
    assert(Request::kSuccess == request.state);

    if (Request::kSuccess != GetVersion(request)) return Process::kUnsupportVersion;
    if (Request::kSuccess != GetMethod(request)) return Process::kUnsupportMethod;
    
    switch (request.method_type) {
        case Request::kGet:
            ProcessMethodGet(request, response);
        case Request::kHead:
        case Request::kPost:
            break;
        default:
            return Process::kUnsupportMethod;
    }
}

int ProcessMethodGet(Request &request, Response &response) {
    strpair path;
    if (!ParseURL(request.url, path)) {
        http_logn("Parser URL error.");
        return Process::kInvalidURL;
    }
    
    HTTPFile file(path);
    const char *data = file.read(&response.data_size);

    if (!data) {
        http_logn("Read file error.");
        return Process::kFileError;
    }

    cout << data << endl;
}

bool ParseURL(const strpair &sp, strpair &out_url) {
    const char *p = sp.beg();
    assert(p);

    out_url.empty();

    if (sp.case_equal_n("http://", 7)) {
        if (p + 7 >= sp.end()) {
            //cerr << "out of range." << endl;
            return false;
        }

        if ( NULL == (p = ParseHostname(p + 7, sp.end())) ) {
            //cerr << "host name!" << endl;
            return false;
        }
    }
    
    if ('/' != *p) {
        //cerr << "no root path." << endl;
        return false;
    }

    out_url.set_str(p, sp.end());
    while (p < sp.end() && *p) {
        if ('?' == *p) {
            out_url.set_end(p);
        }
        if ('#' == *p && !out_url.end()) {
            out_url.set_end(p);
        }
        ++p;
    }
    return true;
}

const char* ParseHostname(const char *p, const char *end) {
    assert(p);
    size_t dot_cnt {0};
    while (p < end && '/' != *p) {
        if ('.' == *p) {
            ++dot_cnt;
        } else if (':' == *p) {
            // nothing
        } else if (!isdigit(*p)) {
            return NULL;
        }
        ++p;
    }
    if (3 != dot_cnt) {
        return NULL;
    }
    return p;
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
    http_logn(path);
}

void test_Process() {
    const char path[] = "index.html";
    strpair sp_path(path, path + strlen(path));
    HTTPFile file(sp_path);

    size_t size 
    const char *request_text = file.read();

    Request request;
    ParseHeader(recvline, request);
    
}



