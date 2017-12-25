#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "typedef.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

int testcase1(void)
{
    ST_TEST st0, st1, st2={1234,12,34,"abc"};
    char str[] = {'T', 'h', 'i', 's'};
    
    st0.a = 100;
    st0.b = 10;
    strcpy(st0.p, str); 
    st1 = st0;
    st0.a = 101;
    st0.p[1] = 'b';

    printf("st0:{%d,%d,%s}\n", st0.a, st0.b, st0.p);
    printf("st1:{%d,%d,%s}\n", st1.a, st1.b, st1.p);
    printf("st2:{%d,%d,%s}\n", st2.a, st2.b, st2.p);
    printf("size:%d\n", sizeof(ST_TEST));
    return 0;
}

int testcase2(void)
{
    unsigned int a = 0x12345678;
    unsigned int *pa = &a;
    unsigned char *pb = (unsigned char *)&a;
    unsigned char b = *pb;

    printf("%#x %#x\n", *pa, b);
    return 0;
}

int testcase3(void)
{
    printf("sizeof pointer: %u, sizeof long: %u, size of ull: %u\r\n", sizeof(char *), sizeof(unsigned long), sizeof(unsigned long long));
    return 0;
}

int testcase4(void)
{
    char *arg[] = {"ls", "-l", NULL};

    if (fork() == 0)
    {
        execvp("ls", arg);
    }

    return 0;
}

#define TEST_STRING	"hello"
#define TEST_STRING_1	"world"
int testcase5(void)
{
    printf(TEST_STRING " zhangyinjun\r\n");
    printf(TEST_STRING TEST_STRING_1"\r\n");
    return 0;
}

#define BUFFER_SIZE	128
#define PORT		8888
void handler(int signo)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("child(%d) is over!\r\n", pid);
    }
}

int testcase6(void)
{
    int listenfd, clientfd;
    int n;
    pid_t pid;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t peerlen;
    char buffer[BUFFER_SIZE];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("create socket fail.");
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)(&serveraddr), sizeof(serveraddr)) < 0)
    {
        perror("bind fail.");
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, 10) == -1)
    {
        perror("listen fail.");
        close(listenfd);
        return -1;
    }

    signal(SIGCHLD, handler);
    peerlen = sizeof(clientaddr);
    while (1)
    {
        if ((clientfd = accept(listenfd, (struct sockaddr *)(&clientaddr), &peerlen)) < 0)
        {
            perror("accept fail.");
            close(listenfd);
            return -1;
        }
        else
        {
            printf("connect from %s:%d\r\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        }

        if ((pid = fork()) < 0)
        {
            perror("fork fail.");
            close(listenfd);
            close(clientfd);
            return -1;
        }
        else if (0 == pid)
        {
            close(listenfd);
            while (1)
            {
                if ((n = recv(clientfd, buffer, BUFFER_SIZE, 0)) < 0)
                {
                    perror("recv fail.");
                    close(clientfd);
                    exit(-1);
                }
                else if (n == 0)
                    break;
                else
                {
                    buffer[n] = '\0';
                    printf("recv mesg: %s[%d]\n", buffer, n);
                }
                
                if (send(clientfd, buffer, n, 0) < 0)
                {
                    perror("send fail.");
                    close(clientfd);
                    exit(-1);
                }
            }
            close(clientfd);
            printf("client closed.\r\n");
            exit(0);
        }
        else
        {
            close(clientfd);
        }
    }

    close(listenfd);
    return 0;
}

int main(int argc, const char *argv[])
{
    return testcase3();
}
