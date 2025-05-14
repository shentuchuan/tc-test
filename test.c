#include <stdio.h>
#include <string.h>

#include "comm.h"
#include "log.h"

int enable_url_dname_filter = 0;

static const unsigned char _dnsValidCharTable[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0X2D, 0, 0x2F,
    0X30, 0X31, 0X32, 0X33, 0X34, 0X35, 0X36, 0X37, 0X38, 0X39, 0x3A, 0,
    0, 0, 0, 0, 0, 0X61, 0X62, 0X63, 0X64, 0X65, 0X66, 0X67,
    0X68, 0X69, 0X6A, 0X6B, 0X6C, 0X6D, 0X6E, 0X6F, 0X70, 0X71, 0X72, 0X73,
    0X74, 0X75, 0X76, 0X77, 0X78, 0X79, 0X7A, 0, 0, 0, 0, 0X5F,
    0, 0X61, 0X62, 0X63, 0X64, 0X65, 0X66, 0X67, 0X68, 0X69, 0X6A, 0X6B,
    0X6C, 0X6D, 0X6E, 0X6F, 0X70, 0X71, 0X72, 0X73, 0X74, 0X75, 0X76, 0X77,
    0X78, 0X79, 0X7A, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0};

int get_dname_len(unsigned char *dname, int pkt_len) {
    int i        = 0;
    int j        = 0;
    int url_flag = 0;

    /* 解析域名 */
    while (i < MAX_DNAME_LEN) {
        if (i >= pkt_len) {
            return -1; /* 这种情况下会发生越界访问 */
        }

        if (dname[i] > 63) {
            return -1;
        }

        if (dname[i] == 0)
            break;

        for (j = i + 1; j <= (i + dname[i]); j++) {
            // printf("%c\n", dname[j]);
            switch (_dnsValidCharTable[dname[j]]) {
                case 0:
                    return -1;
                case 0x2F:
                case 0x3A:
                    if (enable_url_dname_filter) {
                        return -1;
                    }
                    url_flag = 1;
                    continue;
                default:
                    continue;
            }
        }

        if (dname[1] == '-') {
            return -1;
        }

        i += (dname[i] + 1);
    }
    // printf("len %d\n",i);
    if (i >= MAX_DNS_PKT_LEN) {
        return -1;
    }

    if (url_flag) {
        return -2;
    }

    return i;
}

void hexdump(const void *buf, unsigned int len) {
#define LINE_LEN (512)
    unsigned int         i, out, ofs;
    const unsigned char *data = buf;
    char                 line[LINE_LEN]; /* space needed 8+16*3+3+16 == 75 */

    ofs = 0;
    while (ofs < len) {
        /* format the line in the buffer */
        out = snprintf(line, LINE_LEN, "%08X:", ofs);
        for (i = 0; i < 16; i++) {
            if (ofs + i < len)
                snprintf(line + out, LINE_LEN - out, " %02X", (data[ofs + i] & 0xff));
            else
                strcpy(line + out, "   ");
            out += 3;
        }

        for (; i <= 16; i++)
            out += snprintf(line + out, LINE_LEN - out, " | ");

        for (i = 0; ofs < len && i < 16; i++, ofs++) {
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
void trans_to_dname_format(char *src_dn, char *dst_dn) {
    int last_end = 0;
    int i        = 0;
    int dn_len   = strlen(src_dn);

    int lable_num = 0;
    int lable_len = 0;
    for (i = 0; i < dn_len; i++) {
        if (src_dn[i] == '.') {
            lable_len = i - last_end;
            if (lable_num == 0) {
                dst_dn[0] = lable_len;
            } else {
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

    if (src_dn[dn_len - 1] != '.') {
        lable_len = i - last_end;
        if (lable_num == 0) {
            dst_dn[0] = lable_len;
        } else {
            lable_len -= 1;
            dst_dn[last_end + 1] = lable_len;
        }
    }

    // TLog("%x\n", dst_dn[0]);
    dn_len = strlen(dst_dn);
    hexdump(dst_dn, 20);

    return;
}

void test_seg(char *dname) {
#define MAX_DNAME_LEN 256
    struct domain_segs {
        int offset;
        int len;
    };

    struct domain_segs segs[MAX_DNAME_LEN]; // 最多级数<MAX_DNAME_LEN
    int                segs_num = 0;        // 级数
    int                nameLen  = strlen(dname);
    char              *end      = dname + nameLen; // nameLen 包含03www05baidu03com
                                                   // 中第一个03 包括长度为14 dname 指向03

    // printf("1: dnamelen %d, end=%p, len=%d\n", nameLen, end, end-dname);
    int i = 0;

    while (i < nameLen) {                // 解析域名
        if (segs_num >= MAX_DNAME_LEN) { // 异常了
            // TLog("Parse failed!\n");
            return;
        }
        segs[segs_num].offset =
            i;                                  // 记录每一级的位置 offset 03www05baidu03com 指向是03 05 03
        segs[segs_num].len = end - (dname + i); // 记录每一级的位置
        TLog("3: i=%d, len=%d, off=%d, %d, segs_num=%d\n", i, segs[segs_num].len,
             segs[segs_num].offset, dname[i], segs_num);
        segs_num++;
        i += (dname[i] + 1);
    }

    int zone_entry_array_num = 4, zoneid = 0, segid = 0;
    for (i = 1; (i < (zone_entry_array_num)) && (i < segs_num); i++) {
        zoneid = zone_entry_array_num - i;
        segid  = segs_num - i;
        TLog("zone id  = %d, segid=%d\n", zoneid, segid);
    }
}

void test_set_first(rr_list_t *rr_head, int idx) {
    rr_list_t *rr_list;
    int        i = 0;
    list_for_each_entry(rr_list, &rr_head->lnode, lnode) {
        if (i == idx) {
            TLog("\n**************\nSet rr wei=%d\n***************\n",
                 rr_list->cur_weight);
            rr_head->fir = list_first_entry(&rr_head->lnode, rr_list_t, lnode);
            TLog("\n**************\nFir rr wei=%d\n***************\n",
                 rr_head->fir->cur_weight);
        }
        i++;
        // printf("rr wei=%d\n", rr_list->cur_weight);
    }

    return;
}

int test_list_add(rr_list_t *rr_head) {
    int num = 10, i = 0;
    rr_head = malloc(sizeof(rr_list_t));
    if (!rr_head) {
        return -1;
    }
    INIT_LIST_HEAD(&rr_head->lnode);

    rr_list_t *node = NULL;
    for (i = 0; i < num; i++) {
        node = malloc(sizeof(rr_list_t));
        if (!node) {
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

void test_list_for_each(rr_list_t *rr_head) {
    rr_list_t *rr_list;
    list_for_each_entry(rr_list, &rr_head->lnode, lnode) {
        TLog("1: node rr wei=%d\n", rr_list->cur_weight);
        if (rr_list->fir) {
            TLog("2: fir rr wei=%d\n", rr_list->fir->cur_weight);
        }
    }

    return;
}

void test_list() {
    rr_list_t *rr_head = NULL;
    test_list_add(rr_head);

    return;
}
