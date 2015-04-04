#pragma once

class strpair;
class Request;

// TODO:
//void ReadHeader(int sockfd);

void ServeClient(int sockfd);

void ParseAndProcess(const char *text, Request &request);

int ProcessRequest(Request &request);
int ProcessMethodGet(Request &request);
int ProcessMethodHead(Request &request);

int SendResponse(int sockfd, Request &request);

void test_ParseURL();
void test_Process();
