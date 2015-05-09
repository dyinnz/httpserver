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

void AcceptMainLoop(int listenfd, pthread_mutex_t *paccept_mutex);

pthread_mutex_t* CreateProcessMutex();

void AnalyzeProcessExitStatus(int status);

inline bool AddToEpoll(int epollfd, int listenfd, epoll_event *pev);
inline bool DeleteFromEpoll(int epollfd, int listenfd);

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

    if (listen(listenfd, 1000) < 0) { // TODO: 1000 is a temperory number
        http_error("Listen error!\n");
        exit(-1);
    }
    http_log("Begin listen...\n");
    
    // Create mutex
    pthread_mutex_t* paccept_mutex = CreateProcessMutex();
    if (NULL == paccept_mutex) {
        return -1;
    }

    if (0 != pthread_mutex_lock(paccept_mutex)) {
        http_error("Main Test the mutex lock error. %x\n", paccept_mutex);
    } else {
        http_log("Main Test the mutex lock ok\n");
        pthread_mutex_unlock(paccept_mutex);
    }

    // Fork worker process
    int max_workers = 4;    // TODO: should read from configure file
    for (int i = 0; i < max_workers; ++i) {

        if ( 0 == (children[i] = fork()) ) {
            // Child process, return after main loop
            AcceptMainLoop(listenfd, paccept_mutex);
            close(listenfd);
            return 0;
        }
    }

    pthread_mutex_destroy(paccept_mutex);

    // Wait the child process
    int child_pid {0}, status {0};
    while ( (child_pid = waitpid(-1, &status, 0)) > 0) {
        http_log("Child process[%d] exit.\n", child_pid);
        AnalyzeProcessExitStatus(status);
    }

    http_log("The server exit.");
    return 0;
}

void AcceptMainLoop(int listenfd, pthread_mutex_t *paccept_mutex) {
    const int max_fd {1000};
    int used_fd {0};
    epoll_event ev, events[max_fd];

    int epollfd = epoll_create(max_fd);
    if (epollfd < 0) {
        http_error("epoll create error!\n");
        return;
    }

    if (0 != pthread_mutex_lock(paccept_mutex)) {
        http_error("Test the mutex lock error. %x\n", paccept_mutex);
    } else {
        http_log("Test the mutex lock ok\n");
        pthread_mutex_unlock(paccept_mutex);
    }

    for (;;) {

        // get the mutex
        if (0 == used_fd) {
            int lock_ret = pthread_mutex_lock(paccept_mutex);
            if (0 == lock_ret) {
                AddToEpoll(epollfd, listenfd, &ev);
                pthread_mutex_unlock(paccept_mutex);

            } else {
                http_error("Get the mutex lock error. %d\n", lock_ret);
                return;
            }

        } else {
            int lock_ret = pthread_mutex_trylock(paccept_mutex);
            if (0 == lock_ret) {
                AddToEpoll(epollfd, listenfd, &ev);
                pthread_mutex_unlock(paccept_mutex);

            } else if (EBUSY != lock_ret) {
                http_error("Try to get the mutex lock error. %d\n", lock_ret);
                return;
            }
        }

        // wait the events
        int fd_num = epoll_wait(epollfd, events, max_fd, -1);
        if (fd_num < 0) {
            http_error("epoll wait error!\n");
            close(epollfd);
            return;
        }

        http_debug("Please enter to continue! fd_num %d\n", fd_num);

        for (int i = 0; i < fd_num; ++i) {
            if (events[i].data.fd == listenfd) {

                int connfd = accept(listenfd, (sockaddr*)NULL, NULL);
                if (connfd < 0) {
                    http_error("accept error!\n");
                    continue;
                }
                DeleteFromEpoll(epollfd, listenfd);
                http_debug("Get a connect sockfd.\n");

                if (!AddToEpoll(epollfd, connfd, &ev)) {
                    close(connfd);
                    continue;
                }
                used_fd++;

            } else {
                http_debug("--------> Child[%d] Begin severing a connection.\n", getpid());
                ServeClient(events[i].data.fd);
                http_debug("Fininshing severing a connection. <--------\n\n");

                DeleteFromEpoll(epollfd, events[i].data.fd);
                used_fd--;

                close(events[i].data.fd);
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

    // Mutex lockh  
    pthread_mutexattr_t mutex_attr;
    if (0 != pthread_mutexattr_init(&mutex_attr)) {
        http_error("Mutex attribute init error.\n");
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

    http_log("Create mutex OK!\n");
    return paccept_mutex;
}

void AnalyzeProcessExitStatus(int status) {
    if (WIFEXITED(status)) {
        http_debug("exited, status=%d\n", WEXITSTATUS(status));

    } else if (WIFSIGNALED(status)) {
        http_debug("killed by signal %d\n", WTERMSIG(status));

    } else if (WIFSTOPPED(status)) {
        http_debug("stopped by signal %d\n", WSTOPSIG(status));

    } else if (WIFCONTINUED(status)) {
        http_debug("continued\n");
    }
}

bool AddToEpoll(int epollfd, int fd, epoll_event *pev) {
    pev->events = EPOLLIN;
    pev->data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, pev) < 0) {
        http_error("Set epoll listen error!\n");
        close(epollfd);
        return false;
    }
    return true;
}

bool DeleteFromEpoll(int epollfd, int fd) {
    return 0 == epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
}
