#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdio>

#include <vector>
#include <iostream>

#include "serve.h"

using namespace std;

/*----------------------------------------------------------------------------*/

int RunServer();
void EpollTest(int listenfd, pthread_mutex_t *paccept_mutex);
void AcceptMainLoop(int listenfd, pthread_mutex_t *paccept_mutex);
pthread_mutex_t* CreateProcessMutex();

pid_t children[kMaxWorkProcess] {0};

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
    
    // Create mutex
    pthread_mutex_t* paccept_mutex = CreateProcessMutex();
    if (NULL == paccept_mutex) {
        return -1;
    }

    // Fork worker process
    int max_workers = 4;    // TODO: should read from configure file
    for (int i = 0; i < max_workers; ++i) {

        if ( 0 == (children[i] = fork()) ) {
            // Child process, return after main loop
            AcceptMainLoop(listenfd, paccept_mutex);
            return 0;
        }
    }

    pthread_mutex_destroy(paccept_mutex);

    // Wait the child process
    int child_pid {0};
    while ( (child_pid = waitpid(-1, NULL, 0)) > 0) {
        http_log("Child process[%d] exit.\n", child_pid);
    }

    http_log("The server exit.");
    return 0;
}

void AcceptMainLoop(int listenfd, pthread_mutex_t *paccept_mutex) {
    if (listen(listenfd, 10) < 0) { // TODO: 10 is a temperory number
        http_error("Listen error!\n");
        exit(-1);
    }
    http_log("Begin listen...\n");

    for (;;) {
        // test
        int try_ret = pthread_mutex_trylock(paccept_mutex);
        if (EBUSY == try_ret) {
            http_debug("Child[%d] block here!\n", getpid());
        } else if (0 == try_ret) {
            http_debug("Child[%d] try to get mutex lock and then release.\n", getpid());
            pthread_mutex_unlock(paccept_mutex);
        } else {
            http_error("trylock error: %d\n", try_ret);
        }

        pthread_mutex_lock(paccept_mutex);
        http_debug("Child[%d] get mutex lock.\n", getpid());
        int connfd = accept(listenfd, (sockaddr*)NULL, NULL);
        int unlock_ret = pthread_mutex_unlock(paccept_mutex);
        if (unlock_ret != 0) {
            http_debug("the unlock return failed!\n");
        }
        http_debug("Child[%d] release mutex lock.\n", getpid());

        if (connfd < 0) {
            http_error("Accept error!\n");
            continue;
        }
        
        http_debug("--------> Child[%d] Begin severing a connection.\n", getpid());
        ServeClient(connfd);
        close(connfd);
        http_debug("Fininshing severing a connection. <--------\n\n");
    }   
}

void EpollTest(int listenfd, pthread_mutex_t *paccept_mutex) {
    const int max_accept = 1000;
    epoll_event ev, events[max_accept];

    int epollfd = epoll_create(max_accept);
    if (epollfd < 0) {
        http_error("epoll create error!\n");
        return;
    }

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
        http_error("Set epoll listen error!\n");
        close(epollfd);
        return;
    }

    for (;;) {
        int fd_num = epoll_wait(epollfd, events, max_accept, -1);
        if (fd_num < 0) {
            http_error("epoll wait error!\n");
            close(epollfd);
            return;
        }

        for (int i = 0; i < fd_num; ++i) {
            if (events[i].data.fd == listenfd) {

                int connfd = accept(listenfd, (sockaddr*)NULL, NULL);
                if (connfd < 0) {
                    http_error("accept error!\n");
                    continue;
                }

                ev.events = EPOLLIN;
                ev.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
                    http_error("Set epoll listen error!\n");
                    continue;
                }

            } else {
                http_debug("--------> Child[%d] Begin severing a connection.\n", getpid());
                ServeClient(events[i].data.fd);
                close(events[i].data.fd);
                http_debug("Fininshing severing a connection. <--------\n\n");
            }
        }
    }

}

pthread_mutex_t* CreateProcessMutex() {
    // Memory map
    pthread_mutex_t *paccept_mutex = (pthread_mutex_t*)mmap( 0, sizeof(pthread_mutex_t),
                                        PROT_READ | PROT_WRITE,
                                        MAP_ANON | MAP_SHARED, -1, 0);
    if (MAP_FAILED == paccept_mutex) {
        http_error("Memory map error!\n");
        return NULL;
    }

    // Mutex lock
    pthread_mutexattr_t mutex_attr;
    if (0 != pthread_mutexattr_init(&mutex_attr)) {
        http_error("Mutex attribute init error.");
        return NULL;
    }
    if (0 != pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED)) {
        http_error("Set mutex process shared error!\n");
        return NULL;
    }
    if (0 != pthread_mutex_init(paccept_mutex, &mutex_attr)) {
        http_error("Init mutex error!\n");
        return NULL;
    }
    return paccept_mutex;
}
