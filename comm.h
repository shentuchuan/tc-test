#ifndef _TEST_COMM_H_
#define _TEST_COMM_H_

#define MAX_DNS_PKT_LEN 253
#define MIN_DNS_PKT_LEN 1

#ifndef MAX_DNAME_LEN
#define MAX_DNAME_LEN 256
#endif

#include "list.h"

typedef struct rr_list {
    struct list_head lnode;
    int              ori_weight;
    int              cur_weight;
    struct rr_list  *fir;
} rr_list_t;

#endif
