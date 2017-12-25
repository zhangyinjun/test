/****************************************************************/
/*                    version discription                       */
/*  v1. first version                                           */
/*  v2. use 64K RAM(array) to store VRF instead of alloc node   */
/*     dynamically when needed; Directly locate next node       */
/*     instead of traversing the list                           */
/*  v3. define new struct(32bits) to store prio instead of node */
/*     struct(64bits)                                           */
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

typedef struct prio
{
    struct prio *next;
    u32 data;
    u32 index;
}prio_t;

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
#define MAX_LEVEL       32
#define BITS_PER_BYTE   8

u32 node_id = 0;
u32 prio_id = 0;
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
#define SPACE   128*1024*1024
#define ALLOC_NODE(p, i)   \
    do {    \
        if ((node_id * sizeof(node_t) + prio_id * sizeof(prio_t)) >= SPACE)   \
        {   \
            printf("ALLOC_NODE(%u) for rule(%u) fail!\n", node_id, i);  \
            goto done;  \
        }   \
        (p) = (node_t *)(base + node_id * sizeof(node_t));    \
        (p)->u.index = node_id++;  \
    } while(0)

#define ALLOC_PRIO(p, i)    \
    do {    \
        if ((node_id * sizeof(node_t) + prio_id * sizeof(prio_t)) >= SPACE)   \
        {   \
            printf("ALLOC_PRIO(%u) for rule(%u) fail!\n", prio_id, i);  \
            goto done;  \
        }   \
        prio_id++;  \
        (p) = (prio_t *)(base + SPACE - prio_id * sizeof(prio_t));    \
        (p)->index = prio_id;  \
    } while(0)
#endif

rule_t rule_a[MAX_RULE_NUM];
node_t vrf[1<<16] = {{0}};

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
        node_t *p = NULL;
        u16 stepNum = rule_a[i].validlen >> 2;//step len is 4
        u16 bitRemain = rule_a[i].validlen & 0x3;
        u16 k = 0;

        p = &vrf[rule_a[i].vrf];
        while (k < stepNum)
        {        
            u8 stepData = (rule_a[i].data[k>>3] >> ((k & 0x7) << 2)) & 0xf;
            int j = countBits(p->data.flag, stepData);
            
            if ((k+1) == stepMax)
            {
                prio_t *n = NULL;
                
                if (p->data.flag)
                    n = (prio_t *)(base + SPACE - p->data.addr * sizeof(prio_t));
                
                if (!(p->data.flag & (1<<stepData)))
                {
                    prio_t *t = NULL;
                    
                    ALLOC_PRIO(t, i);
                    if (!j)
                    {
                        p->data.addr = t->index;
                        t->next = n;
                    }
                    else
                    {
                        j--;
                        while (j--)
                            n = n->next;
                        
                        t->next = n->next;
                        n->next = t;
                    }
                    n = t;
                    p->data.flag |= 1<<stepData;
                }
                else
                {
                    while (j--)
                        n = n->next;
                }
                n->data = rule_a[i].prio;
            }
            else
            {
                node_t *n = NULL;
                
                if (p->data.flag)
                    n = (node_t *)(base + p->data.addr * sizeof(node_t));
                
                if (!(p->data.flag & (1<<stepData)))
                {
                    node_t *t = NULL;
                    
                    ALLOC_NODE(t, i);
                    if (!j)
                    {
                        p->data.addr = t->u.index;
                        t->next = n;
                    }
                    else
                    {
                        j--;
                        while (j--)
                            n = n->next;
                        
                        t->next = n->next;
                        n->next = t;
                    }
                    n = t;
                    p->data.flag |= 1<<stepData;
                }
                else
                {
                    while (j--)
                        n = n->next;
                }
                p = n;
            }
            k++;
        }

        if (stepNum < stepMax)//process x
        {
            u8 xData = (rule_a[i].data[stepNum>>3] >> ((stepNum & 0x7) << 2)) & 0xf;
            u8 mask = (0xf << bitRemain) & 0xf;
            u8 j;
            u8 new_flag = 0, new_prio = bitRemain;
            prio_t *n = NULL;
            prio_t *t = NULL, *o = NULL;
            u32 prioAddr = 0;

            if (p->data.x_flag)
                n = (prio_t *)(base + SPACE - p->data.x_addr * sizeof(prio_t));

            o = n;

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
                        n->data = rule_a[i].prio;
                        p->data.x_prio &= ~(0x3 << (j<<1));
                        p->data.x_prio |= new_prio << (j<<1);
                    }
                    o = n;
                    n = n->next;
                }
                else if (new_flag & (1<<j))
                {
                    ALLOC_PRIO(t, i);
                    if (!prioAddr)
                        prioAddr = t->index;
                    else
                        o->next = t;

                    t->data = rule_a[i].prio;
                    t->next = n;                    
                    p->data.x_prio |= new_prio << (j<<1);
                    o = t;
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
    /*
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
    */
    printf("total %u nodes.\n", node_id);
    printf("total %u prios.\n", prio_id);
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
        u16 searchVrf = 0;
        u8 data = 0;
        u8 match = 0;
        u32 prio = 0;
        node_t *n = NULL;
        
        printf("Please input search key: ");
        scanf("%s", buf);

        if (buf[0] == 'q')
            return;

        if (strlen(buf) < len[type])
        {
            printf("key min length should be %u\n", len[type]);
            continue;
        }

        if ((etype_ipv4_vrf == type) || (etype_ipv6_vrf == type))
        {
            for (i=0; i<16; i++)
            {
                if (buf[i] == '1')
                    searchVrf |= 1<<i;
            }
            len[type] -= 16;
            temp = buf + 16;
        }
        n = &vrf[searchVrf];

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
                        prio_t *p = NULL;
                        
                        j = countBits(n->data.x_flag, data & 0x7);
                        p = (prio_t *)(base + SPACE - n->data.x_addr * sizeof(prio_t));

                        while (j--)
                            p = p->next;

                        prio = p->data;
                        match = 1;
                    }
                    
                    if ((1<<data) & (n->data.flag))
                    {
                        j = countBits(n->data.flag, data);

                        if ((i+1)==len[type])
                        {
                            prio_t *p = (prio_t *)(base + SPACE - n->data.addr * sizeof(prio_t));

                            while (j--)
                                p = p->next;
                            
                            prio = p->data;
                            match = 1;
                        }
                        else
                        {
                            node_t *p = (node_t *)(base + n->data.addr * sizeof(node_t));

                            while (j--)
                                p = p->next;
                            
                            n = p;
                        }
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
    int i;
    
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

