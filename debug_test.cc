#include <fstream>
#include <iostream>

#include "parse.h"
#include "response.h"

using namespace std;

void test_request(Request &request);
void test_response(Request &request);

void test_parse() {
    ifstream fin("test_text");
    char recvline[kMaxLine] {0};

    fin.seekg(0, ifstream::end);
    int size = fin.tellg();
    fin.seekg(0, ifstream::beg);

    fin.read(recvline, size);
    Request request;
    ParseHeader(recvline, request);

    test_request(request);

    test_response(request);
}

void test_request(Request &request) {
    if (kSuccess != GetMethod(request)) {
        cerr << "Get method error." << endl;
    }
    cout << request.method_type << endl;

    if (kSuccess != GetVersion(request)) {
        cerr << "Get version error." << endl;
    }
    cout << request.major_version << " " << request.minor_version << endl;
}

size_t test_CreateResponse(char *beg, const char *end, Request &request) {
    char *p { beg };
    p = AppendStatusLine(p, request);
    
    const char data[] = {
        "<html>"
        "<title>The first test</title>"
        "<body>"
        "<p>This is a small step</p>"
        "</body></html>"
    };

    p = AppendEntity(p, data, strlen(data));

    if (end == beg) {
        return 0;
    } else {
        return p - beg;
    }
}

void test_response(Request &request) {
    char response[kMaxLine] {0};
    test_CreateResponse(response, response+kMaxLine, request);
    cout << response << endl;
}


