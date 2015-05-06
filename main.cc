#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // read() 

#include "serve.h"

#include <cstdio>
#include <cstring>      // bzero()
#include <ctime>
#include <cstdio>

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

    srand(time(NULL)); 
    int port = rand()%5000 + 5000;
    http_log("The random port is %d\n", port);
    servaddr.sin_port = htons(port); // TODO: 6666 is a temperory number

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
/*
    while (true) {
        int connfd = accept(listenfd, (sockaddr*)NULL, NULL);
        if (connfd < 0) {
            http_error("Accept error!\n");
            return -1;
        }
        ServeClient(connfd);
        close(connfd);
    }
*/
    while (true) {
        int connfd = accept(listenfd, (sockaddr*)NULL, NULL);
        if (connfd < 0) {
            http_error("Accept error!\n");
            continue;
        }
        
        pid_t child = fork();
        if (0 == child) {
            //  Child process
            close(listenfd);

            http_log("Fork a child process to handle a new connection.\n");
            ServeClient(connfd);
            http_log("Child process will exit right now\n");

            return 0;
        }

        http_log("Parent process goes here, close the sockfd of connection.\n");
        // Parent process
        close(connfd);
    }

    return 0;
}

