#include <cstdio>
#include <cstring>      // bzero()

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // read() 

#include "serve.h"

#include <vector>
#include <iostream>

using namespace std;

int RunServer();

/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[]) {
    if (2 == argc) {
        RunServer();

    } else {
        debug_ServeClient(0);
    }
    return 0;
}

int RunServer() {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        http_error("Create socket error!\n");
        return -1;
    }
    http_log("Create socket succeed!\n");

    sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8000); // TODO: 6666 is a temperory number

    if (bind(listenfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        http_error("Bing error!\n");
        return -1;
    }
    http_log("Bind succeed!\n");

    if (listen(listenfd, 10) < 0) { // TODO: 10 is a temperory number
        http_error("Listen error!\n");
        return -1;
    }
    http_log("Begin listen...\n");

    while (true) {
        int connfd = accept(listenfd, (sockaddr*)NULL, NULL);
        if (connfd < 0) {
            http_error("Accept error!\n");
            return -1;
        }
        ServeClient(connfd);
        close(connfd);
    }

    return 0;
}

