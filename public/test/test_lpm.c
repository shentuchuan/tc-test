/* test_lpm.c */
#include "util_lpm.h"
#include "util_public.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// IPv4 测试用例结构
struct test_case_v4 {
    const char *ip;
    int         mask;
    long        result_id;
};

// IPv6 测试用例结构
struct test_case_v6 {
    const char *ip;
    int         mask;
    long        result_id;
};

// 将字符串IP转换为ip_any_t结构
static void str_to_ipany(const char *ip_str, int mask, int is_ipv6,
                         ip_any_t *ip) {
    ip_any_t tmp_ip = {};
    memset(&tmp_ip, 0, sizeof(ip_any_t));
    memset(ip, 0, sizeof(ip_any_t));
    ip->mask    = mask;
    ip->is_ipv6 = is_ipv6;

    if (is_ipv6 == IS_IPV4) {
        inet_pton(AF_INET, ip_str, &tmp_ip.ipv4);
        memcpy((void *)ip->ip_byte, &tmp_ip.ipv4, 4);
    } else {
        inet_pton(AF_INET6, ip_str, &tmp_ip.ipv6);
        memcpy((void *)ip->ip_byte, &tmp_ip.ipv6, 16);
    }
}

// 测试基础功能
static void test_basic_operations() {
    printf("\n=== 开始基础功能测试 ===\n");

    // 1. 创建LPM表
    dns_lpm_hlist_t *lpm_table = dns_lpm_table_create();
    assert(lpm_table != NULL);

    // 2. 准备测试数据
    struct test_case_v4 v4_cases[] = {{"192.168.1.0", 24, 1001},
                                      {"192.168.1.100", 32, 1002},
                                      {"10.0.0.0", 8, 1003}};

    struct test_case_v6 v6_cases[] = {
        {"2001:db8::", 32, 2001}, {"2001:db8:85a3::8a2e:0370:7334", 128, 2002}};

    // 3. 添加规则
    for (int i = 0; i < sizeof(v4_cases) / sizeof(v4_cases[0]); i++) {
        ip_any_t          ip;
        lpm_user_result_t data = {.user_result_id = v4_cases[i].result_id};

        str_to_ipany(v4_cases[i].ip, v4_cases[i].mask, IS_IPV4, &ip);
        assert(0 == dns_lpm_table_addrs_group_add(lpm_table, &ip, &data));
        ip_any_t ip_stack = {};
        dns_lpm_print(&lpm_table->head, &ip_stack, 0, NULL);
    }

    for (int i = 0; i < sizeof(v6_cases) / sizeof(v6_cases[0]); i++) {
        ip_any_t          ip;
        lpm_user_result_t data = {.user_result_id = v6_cases[i].result_id};

        str_to_ipany(v6_cases[i].ip, v6_cases[i].mask, IS_IPV6, &ip);
        assert(0 == dns_lpm_table_addrs_group_add(lpm_table, &ip, &data));
    }

    // 4. 查找测试
    ip_any_t                  lookup_ip;
    dns_lpm_policy_data_out_t result;

    // 测试IPv4精确匹配
    str_to_ipany("192.168.1.100", 32, IS_IPV4, &lookup_ip);
    memset(&result, 0, sizeof(result));
    dns_lpm_table_match(lpm_table, IS_IPV4, lookup_ip.ip_byte, &result);
    assert(result.user_result_cnt == 2); // 应匹配/24和/32
    // assert(result.user_result[0].user_result_id == 1002); // 最长掩码优先

    // 测试IPv6前缀匹配
    str_to_ipany("2001:db8::1", 128, IS_IPV6, &lookup_ip);
    memset(&result, 0, sizeof(result));
    dns_lpm_table_match(lpm_table, IS_IPV6, lookup_ip.ip_byte, &result);
    assert(result.user_result_cnt >= 1);
    assert(result.user_result[0].user_result_id == 2001);

    // 5. 删除测试
    ip_any_t          del_ip;
    lpm_user_result_t del_data = {0};

    // 删除IPv4/32规则
    str_to_ipany("192.168.1.100", 32, IS_IPV4, &del_ip);
    assert(0 == dns_lpm_set_all(LPM_RESULT_TYPE_ID, OP_DEL, &del_ip, &del_data, lpm_table));

    // 验证删除后
    memset(&result, 0, sizeof(result));
    dns_lpm_table_match(lpm_table, IS_IPV4, del_ip.ip_byte, &result);
    assert(result.user_result_cnt == 1); // 只剩/24
    assert(result.user_result[0].user_result_id == 1001);

    // 6. 清理
    dns_lpm_table_free(lpm_table);
    printf("=== 基础功能测试通过 ===\n\n");
}

// 内存泄漏检查测试
static void test_memory_leak() {
    printf("\n=== 开始内存泄漏测试 ===\n");

    dns_lpm_hlist_t *lpm_table = dns_lpm_table_create();
    assert(lpm_table != NULL);

    // 添加并立即删除大量规则
    for (int i = 0; i < 1000; i++) {
        ip_any_t          ip;
        lpm_user_result_t data = {.user_result_id = i};

        str_to_ipany("10.0.0.0", 24, IS_IPV4, &ip);
        assert(0 == dns_lpm_table_addrs_group_add(lpm_table, &ip, &data));

        assert(0 == dns_lpm_set_all(LPM_RESULT_TYPE_ID, OP_DEL, &ip, &data, lpm_table));
    }

    dns_lpm_table_free(lpm_table);
    printf("=== 内存泄漏测试通过 ===\n\n");
}

int main() {
    test_basic_operations();
    test_memory_leak();
    return 0;
}