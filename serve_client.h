#pragma once

class strpair;
class Request;
class Response;

// TODO:
//void ReadHeader(int sockfd);

namespace Process {

enum ProcessError {
    kSuccess,
    kUnsupportVersion, kUnsupportMethod, kInvalidURL, kFileError,
};

}

void ServeClient(int sockfd);

int ProcessRequest(Request &request, Response &response);
int ProcessMethodGet(Request &request, Response &response);
bool ParseURL(const strpair &sp, strpair &out_url);

int SendResponse(int sockfd, Response &response);

const char *ParseHostname(const char *p, const char *end);

void test_ParseURL();
void test_Process();
