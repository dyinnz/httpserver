#include <cstdio>
#include <cstring>      // bzero()
#include <iostream>
#include <fstream>

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // read() 

#include "utility.h"
#include "handle.h"
#include "parse.h"
#include "debug_test.h"
#include "handle.h"

using namespace std;

int RunServer();

/*----------------------------------------------------------------------------*/

int main() {
    //test_parse();
    test_ParseURL();
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
    servaddr.sin_port = htons(8000); // TODO: 6666 is a temperory number

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
        ServeClient(connfd);
        close(connfd);
    }

    return 0;
}



