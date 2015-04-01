#include <cstdio>
#include <cstring>      // bzero()
#include <iostream>
#include <fstream>

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // read() 

#include "utility.h"

using namespace std;

// Read data from socket, and send the message after pasing.
void Handle(int sockfd);

void test_parse();

int RunServer();

int main() {
    test_parse();
    return 0;
    //return RunServer();
}

int RunServer() {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        cerr << "Socket error." << endl;
        return -1;
    }

    sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6666); // TODO: 6666 is a temperory number

    if (bind(listenfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        cerr << "Bind error." << endl;
        return -1;
    }

    if (listen(listenfd, 10) < 0) { // TODO: 10 is a temperory number
        cerr << "Listen error." << endl;
        return -1;
    }

    while (true) {
        int connfd = accept(listenfd, (sockaddr*)NULL, NULL);
        if (connfd < 0) {
            cerr << "Accept error." << endl;  
            return -1;
        }
        Handle(connfd);
    }

    return 0;
}

void Handle(int sockfd) {
    int n {0};
    char recvline[kMaxLine + 1] {0};
    
    while ( (n = read(sockfd, recvline, kMaxLine)) > 0) {
        http_log(recvline);
        ParseText(recvline);
    }

    http_log("Read end.\n");
    
    exit(-1);
}

void test_parse() {
    ifstream fin("test_text");
    char recvline[kMaxLine] {0};

    fin.seekg(0, ifstream::end);
    int size = fin.tellg();
    fin.seekg(0, ifstream::beg);

    fin.read(recvline, size);
    ParseText(recvline);
}




