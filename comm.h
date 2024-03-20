#ifndef _TEST_COMM_H_
#define _TEST_COMM_H_

#define MAX_DNS_PKT_LEN 253
#define MIN_DNS_PKT_LEN 1

#ifndef MAX_DNAME_LEN
#define MAX_DNAME_LEN 256
#endif

#include <stdint.h>

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>


#include "list.h"

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

#pragma pack(push)
#pragma pack(1)

typedef struct query_data {
    uint16_t      q_type;
    uint16_t      q_class;
    unsigned char rr_hdr[0];
} query_data;


typedef struct dns_rrset {
    uint16_t compress_dname;
    uint16_t rr_type;
    uint16_t rr_class;
    uint32_t rr_ttl;
    uint16_t rr_len;
    uint8_t  data[0];
} dns_rrset_t;

typedef struct {
    unsigned id : 16; /*%< query identification number */

#if BYTE_ORDER == BIG_ENDIAN
    /* fields in third byte */
    unsigned qr : 1;     /*%< response flag, 如果是 response,
                     * 那么是 1 */
    unsigned opcode : 4; /*%< purpose of message. 此包的目的.
                     * __ns_opcode */
    unsigned aa : 1;     /*%< authoritive answer */
    unsigned tc : 1;     /*%< truncated message. 如果应答过大,
                     * udp 放不下, 就会被 truncate */
    unsigned rd : 1;     /*%< recursion desired. 是否希望要
                     * 递归查询. */

    /* fields in fourth byte */
    unsigned ra : 1;     /*%< recursion available. 返回包中,
                     * 服务器告知自己能否做递归查询 */
    unsigned unused : 1; /*%< unused bits (MBZ as of 4.9.3a3) */
    unsigned ad : 1;     /*%< authentic data from named */
    unsigned cd : 1;     /*%< checking disabled by resolver */
    unsigned rcode : 4;  /*%< response code, __ns_rcode 例如
                     * 服务器失败是 0010 */
#else
    /* fields in third byte */
    unsigned rd : 1;     /*%< recursion desired */
    unsigned tc : 1;     /*%< truncated message */
    unsigned aa : 1;     /*%< authoritive answer */
    unsigned opcode : 4; /*%< purpose of message */
    unsigned qr : 1;     /*%< response flag */

    /* fields in fourth byte */
    unsigned rcode : 4;  /*%< response code */
    unsigned cd : 1;     /*%< checking disabled by resolver */
    unsigned ad : 1;     /*%< authentic data from named */
    unsigned unused : 1; /*%< unused bits (MBZ as of 4.9.3a3) */
    unsigned ra : 1;     /*%< recursion available */
#endif
    /* remaining bytes */
    unsigned qdcount : 16; /*%< number of question entries */
    unsigned ancount : 16; /*%< number of answer entries */
    unsigned nscount : 16; /*%< number of authority entries */
    unsigned arcount : 16; /*%< number of resource entries */
    //} HEADER;
} dnshdr;


#pragma pack(pop)

typedef enum __ns_type {
    ns_t_invalid    = 0,  /*%< Cookie. */
    ns_t_a          = 1,  /*%< Host address. */
    ns_t_ns         = 2,  /*%< Authoritative server. */
    ns_t_md         = 3,  /*%< Mail destination. */
    ns_t_mf         = 4,  /*%< Mail forwarder. */
    ns_t_cname      = 5,  /*%< Canonical name. */
    ns_t_soa        = 6,  /*%< Start of authority zone. */
    ns_t_mb         = 7,  /*%< Mailbox domain name. */
    ns_t_mg         = 8,  /*%< Mail group member. */
    ns_t_mr         = 9,  /*%< Mail rename name. */
    ns_t_null       = 10, /*%< Null resource record. */
    ns_t_wks        = 11, /*%< Well known service. */
    ns_t_ptr        = 12, /*%< Domain name pointer. */
    ns_t_hinfo      = 13, /*%< Host information. */
    ns_t_minfo      = 14, /*%< Mailbox information. */
    ns_t_mx         = 15, /*%< Mail routing information. */
    ns_t_txt        = 16, /*%< Text strings. */
    ns_t_rp         = 17, /*%< Responsible person. */
    ns_t_afsdb      = 18, /*%< AFS cell database. */
    ns_t_x25        = 19, /*%< X_25 calling address. */
    ns_t_isdn       = 20, /*%< ISDN calling address. */
    ns_t_rt         = 21, /*%< Router. */
    ns_t_nsap       = 22, /*%< NSAP address. */
    ns_t_nsap_ptr   = 23, /*%< Reverse NSAP lookup (deprecated). */
    ns_t_sig        = 24, /*%< Security signature. */
    ns_t_key        = 25, /*%< Security key. */
    ns_t_px         = 26, /*%< X.400 mail mapping. */
    ns_t_gpos       = 27, /*%< Geographical position (withdrawn). */
    ns_t_aaaa       = 28, /*%< Ip6 Address. */
    ns_t_loc        = 29, /*%< Location Information. */
    ns_t_nxt        = 30, /*%< Next domain (security). */
    ns_t_eid        = 31, /*%< Endpoint identifier. */
    ns_t_nimloc     = 32, /*%< Nimrod Locator. */
    ns_t_srv        = 33, /*%< Server Selection. */
    ns_t_atma       = 34, /*%< ATM Address */
    ns_t_naptr      = 35, /*%< Naming Authority PoinTeR */
    ns_t_kx         = 36, /*%< Key Exchange */
    ns_t_cert       = 37, /*%< Certification record */
    ns_t_a6         = 38, /*%< IPv6 address (deprecates AAAA) */
    ns_t_dname      = 39, /*%< Non-terminal DNAME (for IPv6) */
    ns_t_sink       = 40, /*%< Kitchen sink (experimentatl) */
    ns_t_opt        = 41, /*%< EDNS0 option (meta-RR) */
    ns_t_apl        = 42, /*%< Address prefix list (RFC3123) */
    ns_t_svcb       = 64,
    ns_t_https      = 65,
    ns_t_tkey       = 249,    /*%< Transaction key */
    ns_t_tsig       = 250,    /*%< Transaction signature. */
    ns_t_ixfr       = 251,    /*%< Incremental zone transfer. */
    ns_t_axfr       = 252,    /*%< Transfer zone of authority. */
    ns_t_mailb      = 253,    /*%< Transfer mailbox records. */
    ns_t_maila      = 254,    /*%< Transfer mail agent records. */
    ns_t_any        = 255,    /*%< Wildcard match. */
    ns_t_zxfr       = 256,    /*%< BIND-specific, nonstandard. */
    ns_t_a_cname    = 0x0f00, //内部约定 A 配置CNAME 解析结果 类型
    ns_t_aaaa_cname = 0x0f01, //内部约定AAAA 配置CNAME 配置解析结果
    ns_t_max        = 65536
} ns_type;





typedef struct rr_list {
    struct list_head lnode;
    uint8_t          ori_weight;
    uint8_t          cur_weight;
    union {
        //struct in4_addr ans_a;
        //struct in6_addr ans_aaaa;
        uint8_t         data[0];
    } ans;

	struct rr_list   *fir;
} rr_list_t;

typedef struct rr_priority {
    struct list_head lnode;
    uint8_t          priority;
    rr_list_t *      rr_list;
} rr_priority_t;




struct dns_ans_node {
    //atomic_t  num;
    uint32_t  r_code;
    uint16_t  a_or_aaaa_rr_num;
    uint16_t  answer_rr_cnt;
    uint16_t *a_or_aaaa_rr_offset;
    uint16_t *answer_ttl_offset;

    uint8_t            rr_data_len;
    uint8_t            dname_len;
    uint8_t *          data;
    uint32_t           data_len;
    struct query_data *qd;

    uint32_t ttl;

    uint64_t updatesec;
    uint64_t ctl_jiff; //缓存超时应答时，缓存更新控制时间
    uint32_t cache_sec;

    uint8_t cstat;
    uint8_t freaze_ttl_cycle;

    //struct hlist_node list;
    struct list_head  rr_priority_list;
    uint8_t           ans_source_flag;
    uint8_t           is_ecs;
    int64_t expiration_date_jiffs;//缓存有效期
    uint32_t          cache_source_type;//缓存类型
    uint8_t           nxr_type_flag;
};






















#endif

