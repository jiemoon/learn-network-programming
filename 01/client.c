#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

#define PORT 3491
#define MAXDATASIZE 5000

int main(int argc, char const *argv[])
{
    int sockfd, nbytes;
    char buf[1024];
    struct hostent *he;
    struct sockaddr_in srvaddr;

    if (argc != 2) {
        perror("Usage:client hostname\n");
        exit(1);
    }
    if ((he = gethostbyname(argv[1])) == NULL) {
        perror("gethostbyname error\n");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(1);
    }
    bzero(&srvaddr, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(PORT);
    srvaddr.sin_addr = *((struct in_addr *)he->h_addr);
    if (connect(sockfd, (struct sockaddr *) &srvaddr, sizeof(struct sockaddr)) == -1) {
        perror("connect error");
        exit(1);
    }

    if ((nbytes = read(sockfd, buf, MAXDATASIZE)) == -1) {
        perror("read error");
        exit(1);
    }

    buf[nbytes] = '\0';
    printf("read: %s\n", buf);

    printf("Enter You Want to Say: ");
    scanf("%[^\n]", buf);
    if (write(sockfd, buf, 30) == -1) {
        perror("write error");
        exit(1);
    }

    close(sockfd);
    return 0;
}
