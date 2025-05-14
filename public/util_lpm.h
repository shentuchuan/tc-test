#ifndef __UTIL_LPM_H__
#define __UTIL_LPM_H__

#include <stdint.h>
#include <sys/types.h>

#include "util_list.h"

#define IS_IPV6 1
#define IS_IPV4 0

#define UTIL_LPM_MAX_MATCH (8)
#define MAX_INDEX 255
#define MAX_LEVEL_IPV4 4
#define MAX_LEVEL_IPV6 16

typedef uint8_t u_char;

typedef enum LPM_OP {
    OP_QUERY = 1,
    OP_ADD,
    OP_DEL,
    OP_VARY,
    OP_EMPTY,
    OP_SYNC,
    OP_EC_RESTORE,
    OP_EC_STOP,
    OP_EC_EXPORT
} LPM_OP_E;

typedef enum {
    LPM_RESULT_TYPE_ID = 0,
    LPM_RESULT_TYPE_DATA,
    LPM_RESULT_TYPE_MAX
} UTIL_LPM_RESULT_TYPE_E;

#pragma pack(push)
#pragma pack(1)
//---------------------------------------
struct in4_addr {
    union {
        uint8_t  u4_addr8[4];
        uint16_t u4_addr16[2];
        uint32_t u4_addr32;
    } in4_u;

#define s4_addr in4_u.u4_addr8
#define s4_addr16 in4_u.u4_addr16
#define s4_addr32 in4_u.u4_addr32
};

typedef struct ip_any {
    uint8_t is_ipv6; // IS_IPV4/IS_IPV6
    union {
        struct in4_addr ipv4;
        struct in6_addr ipv6;
        uint8_t         ip_byte[16];
    };
    uint8_t mask;
} ip_any_t;

//----------------------------------------
// ip 和 eth 的数据类型
#define ETH_LEN 6
typedef u_char eth_addr_octet[ETH_LEN];

typedef union eth_adr_bytes_s {
    u_char bytes[ETH_LEN];
    struct
    {
        uint32_t int_eth_data;
        uint16_t short_eth_data;
    };
} eth_adr_bytes;

typedef eth_adr_bytes mac_t;
typedef char          macstr_t[18];

//----------------------------------------
typedef struct ip_ary_s {
    union {
        u_char   ip[4];
        uint32_t ip_num;
    };
} ip_ary_t;

typedef ip_ary_t ip_t;
typedef char     ipstr_t[16];

//----------------------------------------
typedef struct ip6_ary_s {
    union {
        u_char   u6_addr8[16];
        uint16_t u6_addr16[8];
        uint32_t u6_addr32[4];
    };
} ip6_ary_t;

typedef ip6_ary_t ip6_t;
typedef ip_any_t  ip_forbid_t;
typedef ip_any_t  ip_query_protocol_t;

typedef struct address_ips {
    uint32_t view_id;
    ip_any_t ip;
} address_ips_t;

#pragma pack(pop)

typedef struct lpm_user_result {
    uint64_t user_result_id;
    // void    *user_result_data;
} lpm_user_result_t;

typedef struct dns_lpm_policy_data {
    struct list_head  list;
    uint8_t           mask;
    lpm_user_result_t user_result;
} dns_lpm_policy_data_t;

typedef struct dns_lpm_policy_data_out {
    uint8_t           user_result_cnt;
    lpm_user_result_t user_result[UTIL_LPM_MAX_MATCH];
} dns_lpm_policy_data_out_t;

typedef struct dns_lpm_policy {
    dns_lpm_policy_data_t *dns_lpm_policy_data;
} dns_lpm_policy_t;

typedef struct dns_lpm_node dns_lpm_node_t;
typedef struct dns_lpm_node {
    struct list_head config_list_ipv4;
    struct list_head config_list_ipv6;
    dns_lpm_node_t  *dns_lpm_node;
} dns_lpm_node_t;

typedef struct dns_lpm_hlist {
    dns_lpm_node_t head;
} dns_lpm_hlist_t;

dns_lpm_hlist_t *dns_lpm_table_create();
void             dns_lpm_table_free(dns_lpm_hlist_t *dns_lpm_hlist);
int              dns_lpm_table_addrs_group_add(dns_lpm_hlist_t *hlist_new, ip_any_t *ip,
                                               lpm_user_result_t *data);
void             dns_lpm_table_match(dns_lpm_hlist_t *p, uint8_t is_ipv6, uint8_t *ip,
                                     dns_lpm_policy_data_out_t *dns_lpm_policy_data_out);
void             dns_lpm_print(dns_lpm_node_t *dns_lpm_node, ip_any_t *ip_stack,
                               uint8_t level, ip_query_protocol_t *filter);
int              dns_lpm_set_all(uint16_t busi_type, uint8_t op, ip_any_t *ip,
                                 lpm_user_result_t *data, dns_lpm_hlist_t *p);
#endif
