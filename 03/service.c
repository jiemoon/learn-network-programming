#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define SERV_PORT 5556
#define LISTENQ 20
#define MAXLINE 1000

void setnonblocking(int sock) 
{
    int opts;
    opts = fcntl(sock, F_GETFL);

    if (opts < 0) {
        perror("fcntl(sock, F_GETFL)");
        exit(1);
    }

    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(sock, F_SETFL, opts)");
        exit(1);
    }
}

int main(int argc, char const *argv[])
{
    int listenfd, epfd, maxi, i, nfds, connfd, sockfd;
    ssize_t n;
    char line[MAXLINE];

    socklen_t clilen;
    // 声明 epoll_event 结构体的变量, ev 用于注册事件,数组用于回传要处理的事件
    struct epoll_event ev, events[20];
    // 生成用于处理accept的epoll专用的文件描述符
    epfd = epoll_create(256);

    struct sockaddr_in srvaddr;
    struct sockaddr_in cliaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // 把 socket 设置为非阻塞方式
    setnonblocking(listenfd);

    // 设置与要处理的事件相关的文件描述符
    ev.data.fd = listenfd;
    // 设置要处理的事件类型
    ev.events = EPOLLIN | EPOLLET;

    // 注册epoll事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    bzero(&srvaddr, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;

    char *local_addr = "127.0.0.1";
    inet_aton(local_addr, &(srvaddr.sin_addr));
    srvaddr.sin_port = htons(SERV_PORT);
    if ((bind(listenfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr))) == -1) {
        perror("bind error");
        exit(1);
    }

    if ((listen(listenfd, LISTENQ)) == -1) {
        perror("listen error");
        exit(1);
    }
    printf("%s\n", "Wait connect...");

    maxi = 0;
    for (;;) {
        // 等待 epoll 事件的发生, 20 每次能处理的事件数, 500 等待I/O事件发生的超时值 ms
        nfds = epoll_wait(epfd, events, 20, 500);
        // 处理所发生的所有事件
        for(i=0; i < nfds; ++i) {
            if (events[i].data.fd == listenfd) {
                connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

                if (connfd == -1) {
                    perror("accept error");
                    exit(1);
                }

                setnonblocking(connfd);
                char *str = inet_ntoa(cliaddr.sin_addr);
                printf("connect from %s\n", str);
                // 设置用于读操作的文件描述符
                ev.data.fd = connfd;
                // 设置用于注测的读操作事件
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            } else if (events[i].events & EPOLLIN) {
                printf("%s: %d\n", "EPOLLIN", i);
                if ((sockfd = events[i].data.fd) < 0) {
                    continue;
                }

                memset(line, 0, sizeof(line));
                if ((n = recv(sockfd, line, MAXLINE, 0)) < 0) {
                    if (n == ECONNREFUSED) {
                        perror("network error");
                        close(sockfd);
                        events[i].data.fd = -1;
                    }
                } else if (n == 0) {
                    perror("read empty");
                    close(sockfd);
                    events[i].data.fd = -1;
                }
                
                line[MAXLINE] = '\0';
                printf("Read from client:(%d) %s\n", n, line);
                // 设置用于写操作的文件描述符
                ev.data.fd = sockfd;
                // 设置用于注测的写操作事件
                ev.events = EPOLLOUT | EPOLLET;
                // 修改sockfd上要处理的事件为EPOLLOUT
                epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
            } else if (events[i].events & EPOLLOUT) {
                printf("%s: %d\n", "EPOLLOUT", i);

                sockfd = events[i].data.fd;
                // printf("Enter You Want to Say: ");
                // scanf("%[^\n]", line);

                write(sockfd, "Hel", 13);
                // 设置用于读操作的文件描述符
                ev.data.fd = sockfd;
                // 设置用于注测的读操作事件
                ev.events = EPOLLIN | EPOLLET;
                // 修改sockfd上要处理的事件为 EPOLLOIN
                epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
            }
        }
    }

    printf("%d\n", listenfd);
    return 0;
}
