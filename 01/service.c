#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MYPORT 3491
#define BACKLOG 5

int main(int argc, char const *argv[])
{
    int sockfd, newfd, nbytes;
    char buf[1024];
    struct sockaddr_in srvaddr;
    struct sockaddr_in cliaddr;
    int sin_size;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(1);
    }
    bzero(&srvaddr, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(MYPORT);
    if ((bind(sockfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr))) == -1) {
        perror("bind error");
        exit(1);
    }
    if ((listen(sockfd, BACKLOG)) == -1) {
        perror("listen error");
        exit(1);
    }
    printf("%s\n", "Wait connect");
    for (;;) {
        sin_size = sizeof(struct sockaddr_in);
        if ((newfd = accept(sockfd, (struct sockaddr *) &cliaddr, (socklen_t *) &sin_size)) == -1) {
            perror("accept error");
            continue;
        }
        printf("server: got connection from %s \n", inet_ntoa(cliaddr.sin_addr));
        if (write(newfd, "Hello, World\n", 12) == -1) {
            perror("write error");
        }

        if ((nbytes = read(newfd, buf, 500)) == -1) {
            perror("read error");
            exit(1);
        }

        buf[nbytes] = '\0';
        printf("read: %s\n", buf);

        close(newfd);
    }
    close(sockfd);
    return 0;
}
