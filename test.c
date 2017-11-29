#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "typedef.h"

int testcase1(void)
{
    ST_TEST st0, st1;
    char str[] = {'T', 'h', 'i', 's'};
    
    st0.a = 100;
    st0.b = 10;
    strcpy(st0.p, str); 
    st1 = st0;
    st0.a = 101;
    st0.p[1] = 'b';

    printf("st0:{%d,%d,%s}\n", st0.a, st0.b, st0.p);
    printf("st1:{%d,%d,%s}\n", st1.a, st1.b, st1.p);
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
    printf("sizeof pointer: %u\r\n", sizeof(char *));
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

int main(int argc, const char *argv[])
{
    return testcase5();
}
