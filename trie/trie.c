/****************************************************************/
/*                    version discription                       */
/*  v1. first version                                           */
/****************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef struct fields
{
    u32 addr;
    u32 x_addr;
    u16 flag;
    u8 x_flag;
    u16 x_prio;
}fields_t;

typedef struct node
{
    struct node *next;
    fields_t data;
    union
    {
        u32 vrf;    //for level 0, store vrf
        u32 index;  //for other levels, store the node's id
    } u;
}node_t;

typedef struct rule
{
    u16 vrf;
    u16 validlen;
    u32 data[4];
    u32 prio;
}rule_t;

typedef enum
{
    etype_ipv4,
    etype_ipv4_vrf,
    etype_ipv6,
    etype_ipv6_vrf,
    etype_max
}ruletype_e;

#define MAX_RULE_NUM    4*1024*1024
#define MAX_LEVEL       33  //last level for prio
#define BITS_PER_BYTE   8

u32 node_id = 0;
#if 0
#define ALLOC_NODE(p)                                   \
    do {                                                \
    if (!((p) = (node_t *)malloc(sizeof(node_t))))      \
    {                                                   \
        printf("ALLOC_NODE(%u) fail!\n", node_id);      \
        goto done;                                      \
    }                                                   \
    memset((void *)(p), 0, sizeof(node_t));             \
    (p)->u.index = node_id++;                             \
    } while(0)

#define FREE_NODE(p)    free((p))
#else
char *base = NULL;
#define SPACE   256*1024*1024
#define ALLOC_NODE(p)   \
    do {    \
        if (node_id * sizeof(node_t) >= SPACE)   \
        {   \
            printf("ALLOC_NODE(%u) fail!\n", node_id);  \
            goto done;  \
        }   \
        (p) = (node_t *)(base + node_id * sizeof(node_t));    \
        (p)->u.index = node_id++;  \
    } while(0)

#define FREE_NODE(p)
#endif

node_t head[MAX_LEVEL] = {{0}};
rule_t rule_a[MAX_RULE_NUM];

static inline int countBits(int n, const int k)
{
    int i;
    int count=0;

    for (i=0; i<k; ++i)
    {
        count=count+(n&0X01);
        n=n>>1;
    }
    return count;
}

void allocate(u32 num, ruletype_e type)
{
    u32 i;
    u8 dataMaxLen[etype_max] = {32, 32, 128, 128};
    u16 stepMax;

    if (type >= etype_max)
    {
        printf("wrong rule type!\n");
        return;
    }

    stepMax = dataMaxLen[type] >> 2;//step len is 4
    for (i=0; i<num; i++)
    {
        node_t *p = &head[0];
        u16 stepNum = rule_a[i].validlen >> 2;//step len is 4
        u16 bitRemain = rule_a[i].validlen & 0x3;
        u16 k = 0;

        while (p->next)
        {
            if (p->next->u.vrf == rule_a[i].vrf)
                break;
            p = p->next;
        }

        if (!p->next)
        {
            ALLOC_NODE(p->next);
            p->next->u.vrf = rule_a[i].vrf;
        }
        
        p = p->next;
        while (k < stepNum)
        {        
            u8 stepData = (rule_a[i].data[k>>3] >> ((k & 0x7) << 2)) & 0xf;
            node_t *n = &head[k+1];
            int j = countBits(p->data.flag, stepData);
            
            while ((n->next) && (n->next->u.index != p->data.addr))
                n = n->next;

            assert(!(!p->data.flag) == !(!n->next));

            if (!(p->data.flag & (1<<stepData)))
            {
                node_t *t = NULL;
                
                ALLOC_NODE(t);
                if (!j)
                    p->data.addr = t->u.index;
                else
                    while (j--)
                        n = n->next;

                t->next = n->next;
                n->next = t;
                
                p->data.flag |= 1<<stepData;
            }
            else
            {
                while (j--)
                    n = n->next;
            }
            p = n->next;
            k++;
            if (k == stepMax)
            {
                p->data.addr = rule_a[i].prio;
            }
        }

        if (stepNum < stepMax)//process x
        {
            u8 xData = (rule_a[i].data[stepNum>>3] >> ((stepNum & 0x7) << 2)) & 0xf;
            u8 mask = (0xf << bitRemain) & 0xf;
            u8 j;
            u8 new_flag = 0, new_prio = bitRemain;
            node_t *n = &head[stepMax];
            node_t *t = NULL;
            u32 prioAddr = 0;

            while ((n->next) && (n->next->u.index != p->data.x_addr))
                n = n->next;

            assert(!(!p->data.x_flag) == !(!n->next));

            for (j=0; j<BITS_PER_BYTE; j++)
            {
                if (((j ^ xData) & ~mask) == 0)
                    new_flag |= 1<<j;
            }

            for (j=0; j<BITS_PER_BYTE; j++)
            {
                if (p->data.x_flag & (1<<j))
                {
                    if (!prioAddr)
                        prioAddr = p->data.x_addr;
                    
                    if ((new_flag & (1<<j)) && (((p->data.x_prio >> (j<<1)) & 0x3) < new_prio))
                    {
                        n->next->data.addr = rule_a[i].prio;
                        p->data.x_prio &= ~(0x3 << (j<<1));
                        p->data.x_prio |= new_prio << (j<<1);
                    }
                    n = n->next;
                }
                else if (new_flag & (1<<j))
                {
                    ALLOC_NODE(t);
                    if (!prioAddr)
                        prioAddr = t->u.index;

                    t->data.addr = rule_a[i].prio;
                    t->next = n->next;
                    n->next = t;
                    p->data.x_prio |= new_prio << (j<<1);
                    n = n->next;
                }
            }
            p->data.x_addr = prioAddr;
            p->data.x_flag |= new_flag;
        }
    }
    
done:
    return;
}

void logAlloc(void)
{
    FILE *pf = NULL;
    int i;
    
    pf = fopen("level_graph.txt", "w");
    if (pf)
    {
        for (i=0; i<MAX_LEVEL; i++)
        {
            node_t *p = head[i].next;
            node_t *n = NULL;

            fprintf(pf, "level[%02d]\n", i);
            while (p)
            {
                fprintf(pf, "{%x,{%x,%x,%x,%x,%x}}\n", p->u.index, p->data.addr, 
                     p->data.x_addr, p->data.flag, p->data.x_flag, p->data.x_prio);
                n = p->next;
                FREE_NODE(p);
                p = n;
            }
        }
        fprintf(pf, "\ntotal %u nodes.\n", node_id);
        fflush(pf);
        fclose(pf);
    }
}

void parseRule(rule_t *rule, char *buf, ruletype_e type)
{
    char *temp = strstr(buf, ",");
    u8 len[etype_max] = {32, 48, 128, 144};
    u8 i;

    if (type >= etype_max)
    {
        printf("wrong rule type!\n");
        return;
    }

    if (!temp)
    {
        printf("wrong rule format!\n");
        return;
    }

    if (len[type] != (u8)(temp - buf))
    {
        printf("wrong rule length for type %d!\n", type);
        return;
    }

    sscanf(temp, ",%x", &rule->prio);
    buf[len[type]] = '\0';
    temp = buf;
    if ((etype_ipv4_vrf == type) || (etype_ipv6_vrf == type))
    {
        for (i=0; i<16; i++,temp++)
        {
            if (*temp == '1')//vrf can not be X
                rule->vrf |= 1<<i;
        }
        len[type] -= 16;
    }

    for (i=0; i<len[type]; i++)
    {
        if (temp[i] == '1')
            rule->data[i/32] |= 1<<(i%32);
        else if ((temp[i] == 'X') || (temp[i] == 'x'))
            break;
    }
    rule->validlen = i;
/*
    if (rule->validlen == len[type])
        printf("vrf:%x data:%08x%08x%08x%08x validlen:%x prio:%x\r\n",
            rule->vrf, rule->data[3], rule->data[2], rule->data[1], rule->data[0], rule->validlen, rule->prio);
*/
}

void readRule(const char *filename, ruletype_e type, u32 *count)
{
    char buf[16+128+24+1] = {0};
    FILE *pf = NULL;
    u32 i = 0;

    pf = fopen(filename, "r");
    if (pf)
    {
        while (fgets(buf, sizeof(buf), pf))
        {
            parseRule(&rule_a[i], buf, type);
            i++;
            if (i >= MAX_RULE_NUM)
                break;
        }

        fclose(pf);
    }
    *count = i;
}

void search(ruletype_e type)
{
    while (1)
    {
        u8 len[etype_max] = {32, 48, 128, 144};
        char buf[512] = {0};
        char *temp = buf;
        int i, j;
        u16 vrf = 0;
        u8 data = 0;
        u8 match = 0;
        u32 prio = 0;
        node_t *n = NULL, *p = NULL;
        
        printf("Please input search key: ");
        scanf("%s", buf);

        if (buf[0] == 'q')
            return;

        if (strlen(buf) < len[type])
        {
            printf("key min length should be %u\n", len[type]);
            return;
        }

        if ((etype_ipv4_vrf == type) || (etype_ipv6_vrf == type))
        {
            for (i=0; i<16; i++)
            {
                if (buf[i] == '1')
                    vrf |= 1<<i;
            }
            len[type] -= 16;
            temp = buf + 16;
        }
        n = &head[0];
        while ((n->next) && (n->next->u.vrf != vrf))
            n = n->next;

        if (!n->next)
            goto done;
        
        n = n->next;

        for (i=0; i<len[type]; i++)
        {
            if (temp[i] == '1')
                data |= 1<<(i&0x3);
            
            if ((i&0x3)==0x3)
            {
                if (n)
                {
                    if ((1<<(data&0x7)) & (n->data.x_flag))
                    {
                        j = countBits(n->data.x_flag, data & 0x7);
                        p = &head[len[type]>>2];

                        while ((p->next) && (p->next->u.index != n->data.x_addr))
                            p = p->next;

                        while ((p->next) && (j--))
                            p = p->next;

                        if (p->next)
                        {
                            prio = p->next->data.addr;
                            match = 1;
                        }
                    }
                    
                    if ((1<<data) & (n->data.flag))
                    {
                        j = countBits(n->data.flag, data);
                        p = &head[(i+1)>>2];
                        while ((p->next) && (p->next->u.index != n->data.addr))
                            p = p->next;

                        while ((p->next) && (j--))
                            p = p->next;

                        if (((i+1)==len[type]) && (p->next))
                        {
                            prio = p->next->data.addr;
                            match = 1;
                        }
                        n = p->next;
                    }
                    else
                        n = NULL;
                }
                data = 0;
            }
        }

    done:
        if (match)
            printf("match : %x\n", prio);
        else
            printf("not match\n");
    }
}

int main(int argc, const char *argv[])
{
    u32 num = 0;
    ruletype_e type = etype_max;
    
    if (argc < 3)
    {
        printf("wrong arg num!\n");
        return -1;
    }

    type = atoi(argv[2]);
    readRule(argv[1], type, &num);
    base = (char *)malloc(SPACE);
    if (!base)
        return -1;

    memset(base, 0, SPACE);
    allocate(num, type);
    search(type);
    logAlloc();
    free(base);
    
    return 0;
}

#if 0
typedef struct
{
    char p;
    u32 b;
} test_t;
int main(int argc, const char *argv[])
{
    u32 a = sizeof(fields_t);
    u32 b = sizeof(test_t);

    printf("%u %u\n", a, b);

    return 0;
}
#endif

