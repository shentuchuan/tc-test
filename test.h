#ifndef _TEST_H_
#define _TEST_H_

#include "comm.h"


void hexdump(const void *buf, unsigned int len);
void test_seg(char *dname);
void trans_to_dname_format(char *src_dn, char *dst_dn);

void test_list();

int test_list_add(rr_list_t *rr_head);

void test_set_first(rr_list_t *rr_head, int idx);


void test_list_for_each(rr_list_t *rr_head);
void cpu_affinity_set(pthread_t thread_id, int cpuCore);

#endif

