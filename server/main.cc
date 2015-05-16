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
#include "configure.h"

using namespace std;

/*----------------------------------------------------------------------------*/

struct ProcessSharedMap {
    pthread_mutex_t accept_mutex;
    bool is_listened;
};

int RunServer();

bool GlobalInit();
ProcessSharedMap* InitProcessSharedMap();
void AcceptMainLoop(int listenfd, ProcessSharedMap *shared);

inline bool AddToEpoll(int epollfd, int listenfd, epoll_event *pev);
inline bool DeleteFromEpoll(int epollfd, int listenfd);

void AnalyzeProcessExitStatus(int status);

pid_t children[kMaxWorkProcess] {0};

/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[]) {
    //if (!GlobalInit()) return -1;
    if (!InitConfigure()) return -1;

    http_log(kDebug, "Test debug\n");

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
    ProcessSharedMap *shared = InitProcessSharedMap();
    if (NULL == shared) {
        return -1;
    }

    // Fork worker process
    int max_workers = 4;    // TODO: should read from configure file
    for (int i = 0; i < max_workers; ++i) {

        if ( 0 == (children[i] = fork()) ) {
            // Child process, return after main loop
            AcceptMainLoop(listenfd, shared);
            close(listenfd);
            return 0;
        }
    }

    // Wait the child process
    int child_pid {0}, status {0};
    while ( (child_pid = waitpid(-1, &status, 0)) > 0) {
        http_log("Child process[%d] exit.\n", child_pid);
        AnalyzeProcessExitStatus(status);
    }

    pthread_mutex_destroy(&shared->accept_mutex);

    http_log("The server exit.\n");
    return 0;
}

void AcceptMainLoop(int listenfd, ProcessSharedMap *shared) {
    const int max_fd {1000};
    int used_fd {0};
    epoll_event ev, events[max_fd];

    pthread_mutex_t *paccept_mutex = &shared->accept_mutex;

    int epollfd = epoll_create(max_fd);
    if (epollfd < 0) {
        http_error("epoll create error!\n");
        return;
    }

    for (;;) {
        http_log("Begin main loop\n");

        // only can one child listen socket
        int lock_ret = pthread_mutex_trylock(paccept_mutex);
        if (EBUSY == lock_ret) {
            // nothing to do
        } else if (0 == lock_ret) {

            if (!shared->is_listened) {
                shared->is_listened = true; 
                AddToEpoll(epollfd, listenfd, &ev);

                http_log("Child[%d] listen socket.\n", getpid());
            }

            if (0 != pthread_mutex_unlock(paccept_mutex)) {
                http_error("Unlock mutex error.\n"); 
            }
                
        } else {
            http_error("Try lock mutex error.\n"); 
            return;
        }

        // wait the events
        int fd_num = epoll_wait(epollfd, events, max_fd, 10000);
        if (fd_num < 0) {
            http_error("epoll wait error!\n");
            close(epollfd);
            return;
        }

        for (int i = 0; i < fd_num; ++i) {
            if (events[i].data.fd == listenfd) {

                http_debug("Child[%d] begin accept.\n", getpid());
                int connfd = accept(listenfd, (sockaddr*)NULL, NULL);

                if (connfd < 0) {
                    http_error("accept error!\n");
                    continue;
                }
                http_debug("Get a connect sockfd.\n");

                int lock_ret = pthread_mutex_trylock(paccept_mutex);
                if (EBUSY == lock_ret) {
                    // nothing to do
                } else if (0 == lock_ret) {
                    DeleteFromEpoll(epollfd, listenfd);
                    shared->is_listened = false;
                    http_log("Child[%d] ends listening socket.\n", getpid());

                    if (0 != pthread_mutex_unlock(paccept_mutex)) {
                        http_error("Unlock mutex error.\n"); 
                    }

                } else {
                    http_error("Try lock mutex error.\n"); 
                    return;
                }

                if (!AddToEpoll(epollfd, connfd, &ev)) {
                    close(connfd);
                    continue;
                }
                used_fd++;

                // for debug
                // sleep(1);
            
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

ProcessSharedMap* InitProcessSharedMap() {
    ProcessSharedMap* shared = (ProcessSharedMap*)mmap( 0, sizeof(pthread_mutex_t),
                                        PROT_READ | PROT_WRITE,
                                        MAP_ANON | MAP_SHARED, -1, 0);
    if (MAP_FAILED == shared) {
        http_error("Memory map error!\n");
        return NULL;
    }
    bzero(shared, sizeof(ProcessSharedMap));

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
    if (0 != pthread_mutex_init(&shared->accept_mutex, &mutex_attr)) {
        http_error("Init mutex error!\n");
        return NULL;
    }

    http_log("Create mutex OK!\n");
    return shared;
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

