#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <unistd.h>

#include "comm.h"
#include "log.h"

#define __USE_GNU
#include <sched.h>

int enable_url_dname_filter = 0;

static const unsigned char _dnsValidCharTable[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X2D, 0, 0x2F,
	0X30, 0X31, 0X32, 0X33, 0X34, 0X35, 0X36, 0X37, 0X38, 0X39, 0x3A, 0, 0, 0, 0, 0,
	0, 0X61, 0X62, 0X63, 0X64, 0X65, 0X66, 0X67, 0X68, 0X69, 0X6A, 0X6B, 0X6C, 0X6D, 0X6E, 0X6F,
	0X70, 0X71, 0X72, 0X73, 0X74, 0X75, 0X76, 0X77, 0X78, 0X79, 0X7A, 0, 0, 0, 0, 0X5F,
	0, 0X61, 0X62, 0X63, 0X64, 0X65, 0X66, 0X67, 0X68, 0X69, 0X6A, 0X6B, 0X6C, 0X6D, 0X6E, 0X6F,
	0X70, 0X71, 0X72, 0X73, 0X74, 0X75, 0X76, 0X77, 0X78, 0X79, 0X7A, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int get_dname_len(unsigned char *dname, int pkt_len)
{
	int i = 0;
	int j = 0;
	int url_flag = 0;

	/* 解析域名 */
	while (i < MAX_DNAME_LEN)
	{
		if (i >= pkt_len)
		{
			return -1; /* 这种情况下会发生越界访问 */
		}

		if (dname[i] > 63)
		{
			return -1;
		}

		if (dname[i] == 0)
			break;

		for (j = i + 1; j <= (i + dname[i]); j++)
		{
			// printf("%c\n", dname[j]);
			switch (_dnsValidCharTable[dname[j]])
			{
			case 0:
				return -1;
			case 0x2F:
			case 0x3A:
				if (enable_url_dname_filter)
				{
					return -1;
				}
				url_flag = 1;
				continue;
			default:
				continue;
			}
		}

		if (dname[1] == '-')
		{
			return -1;
		}

		i += (dname[i] + 1);
	}
	// printf("len %d\n",i);
	if (i >= MAX_DNS_PKT_LEN)
	{
		return -1;
	}

	if (url_flag)
	{
		return -2;
	}

	return i;
}

void hexdump(const void *buf, unsigned int len)
{
#define LINE_LEN (512)
	unsigned int i, out, ofs;
	const unsigned char *data = buf;
	char line[LINE_LEN]; /* space needed 8+16*3+3+16 == 75 */

	ofs = 0;
	while (ofs < len)
	{
		/* format the line in the buffer */
		out = snprintf(line, LINE_LEN, "%08X:", ofs);
		for (i = 0; i < 16; i++)
		{
			if (ofs + i < len)
				snprintf(line + out, LINE_LEN - out,
						 " %02X", (data[ofs + i] & 0xff));
			else
				strcpy(line + out, "   ");
			out += 3;
		}

		for (; i <= 16; i++)
			out += snprintf(line + out, LINE_LEN - out, " | ");

		for (i = 0; ofs < len && i < 16; i++, ofs++)
		{
			unsigned char c = data[ofs];

			if (c < ' ' || c > '~')
				c = '.';
			out += snprintf(line + out, LINE_LEN - out, "%c", c);
		}
		TLog("%s\n", line);
	}
	return;
}

/*
 * exchange "www.baidu.com" ==> "03www05baidu03com"
 */
void trans_to_dname_format(char *src_dn, char *dst_dn)
{
	int last_end = 0;
	int i = 0;
	int dn_len = strlen(src_dn);

	int lable_num = 0;
	int lable_len = 0;
	for (i = 0; i < dn_len; i++)
	{
		if (src_dn[i] == '.')
		{
			lable_len = i - last_end;
			if (lable_num == 0)
			{
				dst_dn[0] = lable_len;
			}
			else
			{
				lable_len -= 1;
				dst_dn[last_end + 1] = lable_len;
			}
			// TLog("2: len%d end%d i%d\n", lable_len, last_end, i);
			lable_num++;
			last_end = i;
			continue;
		}
		dst_dn[i + 1] = src_dn[i];
		// TLog("3: %d %d\n", i, lable_num);
	}

	if (src_dn[dn_len - 1] != '.')
	{
		lable_len = i - last_end;
		if (lable_num == 0)
		{
			dst_dn[0] = lable_len;
		}
		else
		{
			lable_len -= 1;
			dst_dn[last_end + 1] = lable_len;
		}
	}

	// TLog("%x\n", dst_dn[0]);
	dn_len = strlen(dst_dn);
	hexdump(dst_dn, 20);

	return;
}

void cpu_affinity_set(pthread_t thread_id, int cpuCore)
{
	//pthread_t thread_id = pthread_self();
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpuCore, &cpuset);

	int result = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset);
	if (result != 0)
	{
		// 处理设置线程亲和性失败的情况
	}
}

void test_seg(char *dname)
{
#define MAX_DNAME_LEN 256
	struct domain_segs
	{
		int offset;
		int len;
	};

	struct domain_segs segs[MAX_DNAME_LEN]; // 最多级数<MAX_DNAME_LEN
	int segs_num = 0;						// 级数
	int nameLen = strlen(dname);
	char *end = dname + nameLen; // nameLen 包含03www05baidu03com
								 // 中第一个03 包括长度为14 dname 指向03

	// printf("1: dnamelen %d, end=%p, len=%d\n", nameLen, end, end-dname);
	int i = 0;

	while (i < nameLen)
	{ // 解析域名
		if (segs_num >= MAX_DNAME_LEN)
		{ // 异常了
			// TLog("Parse failed!\n");
			return;
		}
		segs[segs_num].offset = i;				// 记录每一级的位置 offset 03www05baidu03com 指向是03 05 03
		segs[segs_num].len = end - (dname + i); // 记录每一级的位置
		TLog("3: i=%d, len=%d, off=%d, %d, segs_num=%d\n", i, segs[segs_num].len, segs[segs_num].offset, dname[i], segs_num);
		segs_num++;
		i += (dname[i] + 1);
	}

	int zone_entry_array_num = 4, zoneid = 0, segid = 0;
	for (i = 1; (i < (zone_entry_array_num)) && (i < segs_num); i++)
	{
		zoneid = zone_entry_array_num - i;
		segid = segs_num - i;
		TLog("zone id  = %d, segid=%d\n", zoneid, segid);
	}
}

void test_set_first(rr_list_t *rr_head, int idx)
{
	rr_list_t *rr_list;
	int i = 0;
	list_for_each_entry(rr_list, &rr_head->lnode, lnode)
	{
		if (i == idx)
		{
			TLog("\n**************\nSet rr wei=%d\n***************\n", rr_list->cur_weight);
			rr_head->fir = list_first_entry(&rr_head->lnode, rr_list_t, lnode);
			TLog("\n**************\nFir rr wei=%d\n***************\n", rr_head->fir->cur_weight);
		}
		i++;
		// printf("rr wei=%d\n", rr_list->cur_weight);
	}

	return;
}

int test_list_add(rr_list_t *rr_head)
{
	int num = 10, i = 0;
	rr_head = malloc(sizeof(rr_list_t));
	if (!rr_head)
	{
		return -1;
	}
	INIT_LIST_HEAD(&rr_head->lnode);

	rr_list_t *node = NULL;
	for (i = 0; i < num; i++)
	{
		node = malloc(sizeof(rr_list_t));
		if (!node)
		{
			return -1;
		}
		node->cur_weight = i + 1;
		list_add(&node->lnode, &rr_head->lnode);
		TLog("list add: wei=%d\n", node->cur_weight);
	}

	test_list_for_each(rr_head);
	test_set_first(rr_head, 3);
	test_list_for_each(rr_head);
	return 0;
}

void test_list_for_each(rr_list_t *rr_head)
{
	rr_list_t *rr_list;
	list_for_each_entry(rr_list, &rr_head->lnode, lnode)
	{
		TLog("1: node rr wei=%d\n", rr_list->cur_weight);
		if (rr_list->fir)
		{
			TLog("2: fir rr wei=%d\n", rr_list->fir->cur_weight);
		}
	}

	return;
}

void test_list()
{
	rr_list_t *rr_head = NULL;
	test_list_add(rr_head);

	return;
}

void dump_ipv4(uint8_t *bytes)
{
#if BYTE_ORDER == BIG_ENDIAN
	TLog("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);
#else
	TLog("%d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
#endif
}

void test_dns_req(u_char *packet)
{
	int ret;
	int i, j;
	int is_locked = 0;
	int old_len;
	dns_rrset_t *rr;
	int answer_rr_cnt;
	uint32_t max_ttl = 0;
	uint32_t min_ttl = 0;
	rr_list_t *rr_list;
	rr_priority_t *rr_priority;
	char *new_data = NULL, *old_data = NULL;
	uint32_t new_len = 0;
	struct query_data *query_data;
	struct dns_ans_node *ans_node = NULL;
	struct dns_ans_node *ans_node_temp = NULL;
	int array_entry_num = 0;
	uint8_t is_ipv6 = 0;
	int new_vid = 0;
	uint32_t ttl_tmp = 0, ttl_switch = 0;
	uint8_t nxr_data_flag = 0;
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	dnshdr *dnsh = NULL;
	uint8_t *dname = NULL;
	uint16_t pkt_len = 0;
	uint16_t dn_len = 0;
	struct query_data *q_data = NULL;
	struct report_stat_info *report_stat_info = NULL;

	struct iphdr a;
	struct udphdr b;
	int dns_packet_all_header_len = sizeof(a) + sizeof(b) + sizeof(dnshdr);

	TLog("\n\n -----------------------------------------------\n");
	TLog("-----------------------------------------------\n");
	TLog("-----------------------------------------------\n");
	// ip_header = (struct ip *)(packet + 14); // 偏移以跳过以太网帧首部
	packet = packet + 14;
	iph = packet;
	udph = (struct udphdr *)((char *)iph + sizeof(struct iphdr));
	dnsh = (dnshdr *)((char *)udph + sizeof(struct udphdr));
	dname = (unsigned char *)((unsigned char *)dnsh + sizeof(dnshdr));
	pkt_len = ntohs(iph->tot_len);
	dn_len = get_dname_len(dname, pkt_len - dns_packet_all_header_len - sizeof(struct query_data));
	q_data = (struct query_data *)(dname + dn_len + 1);
	answer_rr_cnt = ntohs(dnsh->ancount);

	ans_node = malloc(sizeof(ans_node) + pkt_len);

	if (!ans_node)
	{
		TLog("Kalloc ansnode failed\n ------------");
	}

	memset(ans_node, 0, sizeof(ans_node));
	ans_node->data = (uint8_t *)dnsh;
	ans_node->data_len = pkt_len;
	ans_node->qd = q_data;

	ans_node->answer_rr_cnt = answer_rr_cnt;
	ans_node->dname_len = dn_len;
	ans_node->rr_data_len = 4;
	ans_node->updatesec = 1; // 更新的时间jiff
	ans_node->ctl_jiff = 1;	 // 更新的时间jiff
	ans_node->freaze_ttl_cycle = 0;
	INIT_LIST_HEAD(&ans_node->rr_priority_list);

	// 创建新的数据
	{
		rr = (dns_rrset_t *)ans_node->qd->rr_hdr;
		for (i = 0; i < answer_rr_cnt; i++)
		{
			if ((ntohs(rr->compress_dname) & 0xc000) != 0xc000)
			{
				goto FAIL;
			}
			if (rr->rr_type == htons(ns_t_a) || rr->rr_type == htons(ns_t_aaaa))
			{
				ans_node->a_or_aaaa_rr_num++;
			}
			rr = (dns_rrset_t *)((uint8_t *)rr + sizeof(dns_rrset_t) + ntohs(rr->rr_len));
		}
		// ans_node->a_or_aaaa_rr_num = 1;

		ans_node->a_or_aaaa_rr_offset = malloc(sizeof(uint16_t) * ans_node->a_or_aaaa_rr_num);
		if (ans_node->a_or_aaaa_rr_offset == NULL)
		{
			goto FAIL;
		}

		ans_node->answer_ttl_offset = malloc(sizeof(uint16_t) * answer_rr_cnt);
		if (ans_node->answer_ttl_offset == NULL)
		{
			goto FAIL;
		}

		rr = (dns_rrset_t *)ans_node->qd->rr_hdr;
		for (i = 0, j = 0; i < answer_rr_cnt; i++)
		{
			ans_node->answer_ttl_offset[i] = (uint8_t *)&rr->rr_ttl - ans_node->data;
			ttl_tmp = ntohl(*(uint32_t *)(&rr->rr_ttl));
			if (0 == i)
			{
				ans_node->ttl = ttl_tmp;
			}
			else if (ans_node->ttl > ttl_tmp)
			{
				ans_node->ttl = ttl_tmp;
			}
			if (ttl_tmp > max_ttl && ttl_switch)
			{
				rr->rr_ttl = htonl(max_ttl);
				ans_node->ttl = max_ttl;
			}
			if (rr->rr_type == htons(ns_t_a) || rr->rr_type == htons(ns_t_aaaa))
			{
				ans_node->a_or_aaaa_rr_offset[j++] = (uint8_t *)&rr->data - ans_node->data;
			}
			rr = (dns_rrset_t *)((uint8_t *)rr + sizeof(dns_rrset_t) + ntohs(rr->rr_len));
		}

		if (ans_node->a_or_aaaa_rr_num != 0)
		{
			rr_priority = malloc(sizeof(rr_priority_t));
			if (rr_priority == NULL)
			{
				goto FAIL;
			}

			rr_priority->priority = 255;
			list_add(&rr_priority->lnode, &ans_node->rr_priority_list);
		}

		rr = (dns_rrset_t *)ans_node->qd->rr_hdr;
		for (i = 0; i < ans_node->a_or_aaaa_rr_num; i++)
		{
			rr_list = malloc(sizeof(rr_list_t));
			if (rr_list == NULL)
			{
				goto FAIL;
			}

			rr_list->ori_weight = 2;
			rr_list->cur_weight = 2;

			memcpy(rr_list->ans.data, ans_node->data + ans_node->a_or_aaaa_rr_offset[i], ans_node->rr_data_len);
			if (NULL == rr_priority->rr_list)
			{
				rr_priority->rr_list = rr_list;
				INIT_LIST_HEAD(&rr_priority->rr_list->lnode);
			}
			else
			{
				list_add(&rr_list->lnode, &rr_priority->rr_list->lnode);
			}
		}
	}

	// 遍历链表，查询响应
	struct dns_ans_node *ans = ans_node;
	int q_num = 10;
	int req = 0;
	struct list_head *list_head;

	TLog("---------------------------------\n");
	list_for_each_entry(rr_priority, &ans->rr_priority_list, lnode)
	{
		j = 0;
		rr_list = rr_priority->rr_list;
		list_head = &rr_priority->rr_list->lnode;

		TLog("Loop time %d; ans-num %d; %p ,%p %p\n", j++, req, rr_list, list_first_entry(list_head, rr_list_t, lnode),
			 list_head);
		list_for_each_entry(rr_list, list_head, lnode)
		{
			// memcpy(data + ans->a_or_aaaa_rr_offset[i++], rr_list->ans.data, ans->rr_data_len);
			TLog("Loop time %d; ans-num %d; %p %p\n", j++, req, rr_list, rr_list->lnode);
			hexdump(rr_list->ans.data, ans->rr_data_len);
			dump_ipv4((unsigned char *)rr_list->ans.data);
		}
	}

	TLog("---------------------------------\n");
	// char data[128] = {};
	for (req = 0; req < q_num; req++)
	{
		int j = 0;
		list_for_each_entry(rr_priority, &ans->rr_priority_list, lnode)
		{
			if (rr_priority->rr_list != NULL)
			{
				list_head = &rr_priority->rr_list->lnode;
				rr_list = rr_priority->rr_list;
				rr_list->cur_weight -= 1;
				if (rr_list->cur_weight == 0)
				{
					rr_list->cur_weight = rr_list->ori_weight;
					rr_priority->rr_list = list_first_entry(list_head, rr_list_t, lnode);
				}

				// memcpy(data + ans->a_or_aaaa_rr_offset[i++], rr_list->ans.data, ans->rr_data_len);
				TLog("Loop time %d; ans-num %d; %p\n", j++, req, rr_list);
				hexdump(rr_list->ans.data, ans->rr_data_len);
				dump_ipv4((unsigned char *)rr_list->ans.data);

				list_for_each_entry(rr_list, list_head, lnode)
				{
					// memcpy(data + ans->a_or_aaaa_rr_offset[i++], rr_list->ans.data, ans->rr_data_len);
					TLog("Loop time %d; ans-num %d; %p\n", j++, req, rr_list);
					hexdump(rr_list->ans.data, ans->rr_data_len);
					dump_ipv4((unsigned char *)rr_list->ans.data);
				}
			}
		}

		TLog("====================Resp %d Done===================\n", req);
	}

	TLog("---------------------------------\n");
	list_for_each_entry(rr_priority, &ans->rr_priority_list, lnode)
	{
		j = 0;
		rr_list = rr_priority->rr_list;
		list_head = &rr_priority->rr_list->lnode;

		TLog("Loop time %d; ans-num %d; %p ,%p %p\n", j++, req, rr_list, list_first_entry(list_head, rr_list_t, lnode),
			 list_head);
		list_for_each_entry(rr_list, list_head, lnode)
		{
			// memcpy(data + ans->a_or_aaaa_rr_offset[i++], rr_list->ans.data, ans->rr_data_len);
			TLog("Loop time %d; ans-num %d; %p\n", j++, req, rr_list);
			hexdump(rr_list->ans.data, ans->rr_data_len);
			dump_ipv4((unsigned char *)rr_list->ans.data);
		}
	}

	return;
FAIL:

	TLog("-----------------FAILED--------------------------\n");
	if (ans_node)
	{
		free(ans_node);
	}
	return;
}

int test_pcap_req(void)
{
	pcap_t *handle;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr header;
	const u_char *packet;

	// 打开 pcap 文件
	handle = pcap_open_offline("test.pcap", errbuf);
	if (handle == NULL)
	{
		TLog("无法打开 pcap 文件: %s\n", errbuf);
		return 1;
	}

	// 逐个读取数据包并进行处理
	int i = 0;
	while (packet = pcap_next(handle, &header))
	{
		if (i != 0)
		{
			test_dns_req(packet);
		}
		i++;
	}

	// 关闭 pcap 文件
	pcap_close(handle);

	return 0;
}
