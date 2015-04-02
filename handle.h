#pragma once

class Request;
class strpair;

// TODO:
//void ReadHeader(int sockfd);


void ServeClient(int sockfd);

void HandleRequest(Request &request);
void HandleMethodGet(Request &request);
bool ParseURL(const strpair &sp, strpair &out_url);

const char *ParseHostname(const char *p, const char *end);

void test_ParseURL();
