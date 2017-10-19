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

int main(int argc, const char *argv[])
{
    return testcase2();
}
