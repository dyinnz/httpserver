#include "utility.h"

#include <sys/socket.h> // socket()
#include <sys/wait.h>
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // read() 
#include <arpa/inet.h>

#include <pthread.h>

static void* connect_thread(void *port);
void build_connect(int port);

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Please enter port\n");
        return -1;
    }
    int port = atoi(argv[1]);
/*
    int max_connect = 100;
    if (argc == 3) {
        max_connect = atoi(argv[2]);
    }
*/
    int thread_num = 16;
    pthread_t thread_connect[thread_num];
    for (int i = 0; i < thread_num; ++i) {
        if (0 != pthread_create(&thread_connect[i], NULL, &connect_thread, &port)) {
            printf("create thread error!\n");
            return -1;
        }
        printf("pthread[%d] begin...\n", i);
    }

    for (int i = 0; i < thread_num; ++i) {
        pthread_join(thread_connect[i], (void**)NULL);
    }

    return 0;
}

void* connect_thread(void *port) {
    int max_connect = 100;
    while (max_connect--) {
        printf("times %d\r", max_connect);
        build_connect(*(int*)port);
    }
    return NULL;
}
    
void build_connect(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Init socket failed!\n");
        return;
    }
    printf("Init socket OK\t");

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0) {
        printf("inet_pton error!\n");
    }
    printf("inet_pton ok! port:%d\t", port);

    if (0 != connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr))) {
        printf("Connect failed!\n");
        return;
    }
    printf("Connect OK\n");
    
    const char text[] = {
        "GET /index.html HTTP/1.1\r\n\r\n"
    };
    write(sockfd, text, sizeof(text));
    
    char buff[4096];
    read(sockfd, buff, 4096);

    printf("Get the response: %s\n", buff);
    close(sockfd);
}

