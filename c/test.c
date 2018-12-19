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
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

typedef struct{
    int a:24;
    int b:1;
    int c:1;
    int d:6;
} BITSTR; 

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
    BITSTR s = {.a=0xabcdef,.b=1,.c=0,.d=0x12};

    printf("%#x %#x\n", *pa, b);
    memcpy(&a, &s, sizeof(unsigned int));
    printf("%#x\n", *pa);
    return 0;
}

int g_test = 90;
int testcase3(void)
{
    printf("sizeof int: %u, sizeof long: %u, size of ull: %u, g_test: %d\r\n", sizeof(int), sizeof(unsigned long), sizeof(unsigned long long), g_test);
    return -5;
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
#define NAME(ID)	"file_"#ID
#define NAME2(ID)	file_##ID
int testcase5(void)
{
	char *file_0 = "abc.c", *file_1 = "def.c";
    char d = -1;
    unsigned char e = -1;
    printf(TEST_STRING" zhangyinjun\r\n");
    printf(TEST_STRING TEST_STRING_1"\r\n");
    printf(NAME(100)"\n");
	printf("%s\n", NAME2(1));
    printf("%2x%2x%2x%2x\n", 1, 0x20, 0x100, 0x200);
    printf("%d %d\n", d, e);
    if (d == 255) printf("wwww\n");
    return 0;
}

#define BUFFER_SIZE	1024
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

int testcase7(void)//heap sort
{
    int arr[10] = {0};
    int i = 0;
    int temp;
    int j = 0;

    srand(time(0));
    for (j=0; j<10; j++)
    {
        arr[j] = rand() % 100;
        i = j;
        while ((i>0) && (arr[i]<arr[(i-1)/2]))
        {
            temp = arr[i];
            arr[i] = arr[(i-1)/2];
            arr[(i-1)/2] = temp;
            i = (i - 1) / 2;
        }
    }

    for (j=9; j>0; j--)
    {
        temp = arr[0];
        arr[0] = arr[j];
        arr[j] = temp;

        i = 0;
        while ((i*2+1)<j)
        {
            int k = i * 2 + 1;

            if ((k+1<j) && (arr[k+1] < arr[k])) k++;
            if (arr[k] < arr[i])
            {
                temp = arr[k];
                arr[k] = arr[i];
                arr[i] = temp;
            }
            i = k;
        }
    }

    for (i=0; i<10; i++)
        printf("%d ", arr[i]);

    printf("\n");
}

#define MAX_PHASE_NUM 200 
#define MAX_DAC_NUM 200
typedef struct
{
   int err[MAX_DAC_NUM];
   int bits[MAX_DAC_NUM]; 
} data_s;

int testcase8(char *fn_in)
{
    FILE *fp_in = NULL;
    data_s *pData = NULL;
    char line[100] = {0};
    int i, j;
    int phase_cnt = 0, dac_cnt = 0;

    pData = (data_s *)malloc(MAX_PHASE_NUM * sizeof(data_s));
    if (!pData) goto done;

    memset(pData, 0, MAX_PHASE_NUM * sizeof(data_s));

    fp_in = fopen(fn_in, "r");    
    if (!fp_in) {printf("open %s fail.\n", fn_in); goto done;}

    while (fgets(line, sizeof(line), fp_in))
    {
        int phase_id = 0, dac_id = 0, err = 0, bits = 0;

        if ((line[0] == '#') || (line[0] == '\n')) continue;
        sscanf(line, "%3d,%4d,%7d,%14d", &phase_id, &dac_id, &err, &bits);
        if ((phase_id < MAX_PHASE_NUM) && (dac_id < MAX_DAC_NUM))
        {
            pData[phase_id].err[dac_id] = err;
            pData[phase_id].bits[dac_id] = bits;
            if (dac_id > dac_cnt) dac_cnt = dac_id;
            if (phase_id > phase_cnt) phase_cnt = phase_id;
        }
        //printf("%d,%d\n", pData[phase_id].err[dac_id], pData[phase_id].bits[dac_id]);
    }

    printf("eye diagram %d x %d:\n", phase_cnt+1, dac_cnt+1);
    for (i = dac_cnt; i >= 0; i--)
    {
        printf("%4d:", i-dac_cnt/2);
        for (j = 0; j <= phase_cnt; j++) 
        {
            char c;
            int val = pData[j].err[i];

            if (val <= 0) c = ' ';
            else if (val < 30) c = '*';
            else 
            {
                if (j == phase_cnt/2) c = '|';
                else if (i == dac_cnt/2) c = '=';
                else c = '#';
            }
            printf("%c", c);
        }
        printf("\n");
    }

done:
    if (pData) free(pData);
    if (fp_in) fclose(fp_in);
    return 0;
}

int testcase9(void)
{
    char buf[BUFFER_SIZE] = {0};
    struct sockaddr_in sockaddr = {0};
    struct sockaddr_ll listenaddr = {0};
    int i, len, addr_len;
    int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));

    if (sock < 0)
    {
        perror("create sock fail");
        return -1;
    }
    listenaddr.sll_family = PF_PACKET;
    listenaddr.sll_ifindex = 2;
    listenaddr.sll_protocol = htons(ETH_P_IP);
    if (bind(sock, (struct sockaddr *)&listenaddr, sizeof(listenaddr)) < 0)
    {
        perror("bind fail");
        close(sock);
        return -1;
    }

    while (1)
    {
        addr_len = sizeof(listenaddr);
        len = recvfrom(sock, buf, BUFFER_SIZE, 0, (struct sockaddr *)&listenaddr, &addr_len);
        //printf("receive packet from %s, len %d:\n", inet_ntoa(sockaddr.sin_addr), len);
        printf("%u, %04x, %d, %u, %u, %u\n", listenaddr.sll_family, ntohs(listenaddr.sll_protocol),
            listenaddr.sll_ifindex, listenaddr.sll_hatype, listenaddr.sll_pkttype, listenaddr.sll_halen);
        for (i=0; i<len; i++) printf("%02x ", (unsigned char)buf[i]);
        printf("\n");
    }

    close(sock);
    return 0;
}


int main(int argc, const char *argv[])
{
    //char *fn = "data1.log";
    //if (argc > 1) fn = argv[1];
    //return testcase8(fn);
    return testcase9();
}
