#include "util_lpm.h"
#include "util_public.h"

#define IN6_ADDR_SIZE 16 /* sizeof in6 addr (byte) */

/* dns_lpm_hlist_t *          dns_lpm_hlist = NULL;
dns_lpm_policy_data_out_t *dns_lpm_policy_data_out_percpu; */

void ipv4_addr_copy(struct in4_addr *a1, const struct in4_addr *a2)
{
    memcpy(a1, a2, sizeof(struct in4_addr));
}

void ipv4_addr_prefix(struct in4_addr *pfx, struct in4_addr *addr, int plen)
{
    int o = plen >> 3, b = plen & 0x7;

    memset(pfx->s4_addr, 0, sizeof(pfx->s4_addr));
    memcpy(pfx->s4_addr, addr, o);

    if (b != 0)
    {
        pfx->s4_addr[o] = addr->s4_addr[o] & ~(0xff >> b);
    }
}

void ipv6_addr_copy(struct in6_addr *a1, const struct in6_addr *a2)
{
    memcpy(a1, a2, sizeof(struct in6_addr));
}

void ipv6_addr_prefix(struct in6_addr *pfx, const struct in6_addr *addr,
                      int plen)
{
    /* caller must guarantee 0 <= plen <= 128 */
    int o = plen >> 3, b = plen & 0x7;

    memset(pfx->s6_addr, 0, sizeof(pfx->s6_addr));
    memcpy(pfx->s6_addr, addr, o);
    if (b != 0)
        pfx->s6_addr[o] = addr->s6_addr[o] & (0xff00 >> b);
}

int inet_ntop4(const unsigned char *src, unsigned char *dst,
               unsigned char size)
{
    const unsigned int MIN_SIZE = 16; /* space for 255.255.255.255\0 */
    int n = 0;
    unsigned char *next = dst;

    if (size < MIN_SIZE)
    {
        return -1;
    }
    do
    {
        unsigned char u = *src++;
        if (u > 99)
        {
            *next++ = '0' + u / 100;
            u %= 100;
            *next++ = '0' + u / 10;
            u %= 10;
        }
        else if (u > 9)
        {
            *next++ = '0' + u / 10;
            u %= 10;
        }
        *next++ = '0' + u;
        *next++ = '.';
        n++;
    } while (n < 4);
    *--next = 0;
    return 0;
}

int inet_ntop6(const unsigned char *src, unsigned char *dst,
               unsigned char size)
{
    /*
     * the max size 46 (include last '\0') as:
     * ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255
     *
     * and the min size 4 (include last '\0') as:
     * ::1
     * a v6 loopback
     *
     * the max dst size 46 is always a safe value
     */

    unsigned char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"],
        *tp;
    struct
    {
        int base, len;
    } best = {-1, 0}, cur = {-1, 0};

    /*
     * one word: 0xffff
     * word count = in6 addr size / word size (16bits)
     */

#define WCOUNT (IN6_ADDR_SIZE / sizeof(uint16_t))

    unsigned int words[WCOUNT];

    int i;
    const unsigned char *next_src, *src_end;
    unsigned int *next_dest;

    /*
     * Preprocess:
     *  Copy the input (bytewise) array into a wordwise array.
     *  Find the longest run of 0x00's in src[] for :: shorthanding.
     */
    next_src = src;
    src_end = src + IN6_ADDR_SIZE;
    next_dest = words;
    i = 0;
    do
    {
        unsigned int next_word = (unsigned int)*next_src++;
        next_word <<= 8;
        next_word |= (unsigned int)*next_src++;
        *next_dest++ = next_word;

        if (next_word == 0)
        {
            if (cur.base == -1)
            {
                cur.base = i;
                cur.len = 1;
            }
            else
            {
                cur.len++;
            }
        }
        else
        {
            if (cur.base != -1)
            {
                if (best.base == -1 || cur.len > best.len)
                {
                    best = cur;
                }
                cur.base = -1;
            }
        }

        i++;
    } while (next_src < src_end);

    if (cur.base != -1)
    {
        if (best.base == -1 || cur.len > best.len)
        {
            best = cur;
        }
    }
    if (best.base != -1 && best.len < 2)
    {
        best.base = -1;
    }

    /*
     * Format the result.
     */
    tp = tmp;
    for (i = 0; i < WCOUNT;)
    {
        /* Are we inside the best run of 0x00's? */
        if (i == best.base)
        {
            *tp++ = ':';
            i += best.len;
            continue;
        }
        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0)
        {
            *tp++ = ':';
        }
        /* Is this address an encapsulated IPv4? */
        if (i == 6 && best.base == 0 &&
            (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
        {
            if (!inet_ntop4(src + 12, tp, sizeof tmp - (tp - tmp)))
            {
                return -1;
            }
            tp += strlen((char *)tp);
            break;
        }
        tp += snprintf((char *)tp, sizeof tmp - (tp - tmp), "%x", words[i]);
        i++;
    }
    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) == WCOUNT)
    {
        *tp++ = ':';
    }
    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((tp - tmp) > size)
    {
        return -1;
    }
    strcpy((char *)dst, (const char *)tmp);
    return 0;
}

void dns_lpm_free(dns_lpm_node_t *dns_lpm_node);

int ipaddr_prefix_recalc(ip_any_t *ip)
{
    struct in6_addr ipv6 = {};
    struct in4_addr ipv4 = {};
    memset(&ipv6, 0, sizeof(ipv6));
    memset(&ipv4, 0, sizeof(ipv4));

    if (ip->is_ipv6 == IS_IPV6)
    {
        ipv6_addr_prefix(&ipv6, &(ip->ipv6), ip->mask);
        ipv6_addr_copy(&(ip->ipv6), &ipv6);
    }
    else if (ip->is_ipv6 == IS_IPV4)
    {
        ipv4_addr_prefix(&ipv4, &(ip->ipv4), ip->mask);
        ipv4_addr_copy(&(ip->ipv4), &ipv4);
    }

    return 0;
}

dns_lpm_node_t *alloc_single_hash_level(void)
{
    int i = 0;
    dns_lpm_node_t *single_hash_head = NULL;

    single_hash_head =
        (dns_lpm_node_t *)UTIL_MALLOC(sizeof(dns_lpm_node_t) * (MAX_INDEX + 1));
    if (!single_hash_head)
    {
        util_log_error("malloc error in");
        return NULL;
    }
    for (i = 0; i <= MAX_INDEX; i++)
    {
        INIT_LIST_HEAD(&single_hash_head[i].config_list_ipv4);
        INIT_LIST_HEAD(&single_hash_head[i].config_list_ipv6);
        single_hash_head[i].dns_lpm_node = NULL;
    }
    return single_hash_head;
}

int dns_lpm_set_ip_policy_info(uint16_t result_type, uint8_t op,
                               lpm_user_result_t *data,
                               dns_lpm_policy_data_t *dns_lpm_policy_data,
                               ip_any_t *ip)
{
    if (op != OP_ADD && op != OP_DEL)
    {
        util_log_error("op error!");
        return -1;
    }

    dns_lpm_policy_data->mask = ip->mask;

    if (OP_ADD == op)
    {
        dns_lpm_policy_data->user_result = *(lpm_user_result_t *)data;
    }
    else if (OP_DEL == op)
    {
        memset(&dns_lpm_policy_data->user_result, 0, sizeof(lpm_user_result_t));
        dns_lpm_policy_data->mask = 0;
    }

    return 0;
}

int __dns_lpm_set_ip_policy(uint16_t busi_type, uint8_t op,
                            lpm_user_result_t *data,
                            dns_lpm_node_t *dns_lpm_node, ip_any_t *ip)
{
    dns_lpm_policy_data_t *dns_lpm_policy_data = NULL;
    dns_lpm_policy_data_t *dns_lpm_policy_data_new = NULL;

    int ret = 0;

    if (ip->is_ipv6 != IS_IPV4 && ip->is_ipv6 != IS_IPV6)
    {
        util_log_error("is_ipv6 != 4 or 6\n");
        return -1;
    }

    // 已经存在节点
    list_for_each_entry(dns_lpm_policy_data,
                        ip->is_ipv6 == IS_IPV4
                            ? &(dns_lpm_node->config_list_ipv4)
                            : &(dns_lpm_node->config_list_ipv6),
                        list)
    {
        if (dns_lpm_policy_data->mask == ip->mask)
        {
            ret = dns_lpm_set_ip_policy_info(busi_type, op, data, dns_lpm_policy_data,
                                             ip);
            if (op == OP_DEL)
            {
                list_del(&dns_lpm_policy_data->list);
                UTIL_FREE(dns_lpm_policy_data);
                ret = 0;
                goto out;
            }
            goto out;
        }
    }

    if (op == OP_DEL)
    {
        ret = 0;
        goto out;
    }

    // 还未存在
    dns_lpm_policy_data_new =
        (dns_lpm_policy_data_t *)UTIL_MALLOC(sizeof(dns_lpm_policy_data_t));
    if (!dns_lpm_policy_data_new)
    {
        ret = -1;
        goto out;
    }
    memset(dns_lpm_policy_data_new, 0, sizeof(dns_lpm_policy_data_t));
    INIT_LIST_HEAD(&dns_lpm_policy_data_new->list);

    ret = dns_lpm_set_ip_policy_info(busi_type, op, data, dns_lpm_policy_data_new,
                                     ip);
    if (ret < 0)
    {
        UTIL_FREE(dns_lpm_policy_data_new);
        goto out;
    }
    list_for_each_entry(dns_lpm_policy_data,
                        ip->is_ipv6 == IS_IPV4
                            ? &(dns_lpm_node->config_list_ipv4)
                            : &(dns_lpm_node->config_list_ipv6),
                        list)
    {
        if (ip->mask > dns_lpm_policy_data->mask)
        {
            list_add(&dns_lpm_policy_data_new->list,
                     &dns_lpm_policy_data->list); // 掩码大的在后 优先级高
            goto out;
        }
    }
    list_add(&dns_lpm_policy_data_new->list,
             ip->is_ipv6 == IS_IPV4 ? &(dns_lpm_node->config_list_ipv4)
                                    : &(dns_lpm_node->config_list_ipv6));

out:
    return ret;
}

int _dns_lpm_set_ip_policy(uint16_t busi_type, uint8_t op, ip_any_t *ip,
                           lpm_user_result_t *data,
                           dns_lpm_node_t *dns_lpm_node, uint8_t level)
{
    dns_lpm_node_t *tmp_hash_head = NULL;
    int ret = 0;
    uint8_t extra = 0;
    int i = 0, index = 0;
    int alloc_node = 0;

    dns_lpm_node_t *new_hash_head = NULL;

    if (dns_lpm_node->dns_lpm_node == NULL)
    {
        new_hash_head = alloc_single_hash_level();
        alloc_node = 1;
    }
    else
    {
        new_hash_head = dns_lpm_node->dns_lpm_node;
    }

    tmp_hash_head = &(new_hash_head[(ip->ip_byte[level])]);
    if ((level + 1) * 8 < ip->mask)
    { // 递归往深处走
        _dns_lpm_set_ip_policy(busi_type, op, ip, data, tmp_hash_head, level + 1);
    }
    else if ((level + 1) * 8 >= ip->mask)
    {
        extra = (~(0xff << ((level + 1) * 8 - ip->mask)) &
                 0xff); // 掩码不为整数 还需额外配置的节点个数
        for (i = 0; i <= extra; i++)
        {
            index = ip->ip_byte[level] + i;
            if (index > MAX_INDEX)
            {
                break;
            }
            tmp_hash_head = &(new_hash_head[index]);
            ret = __dns_lpm_set_ip_policy(busi_type, op, data, tmp_hash_head, ip);
            if (ret < 0)
            {
                return -1;
            }
        }
    }
    if (alloc_node == 1)
    {
        dns_lpm_node->dns_lpm_node = new_hash_head;
    }
    return 0;
}

int dns_lpm_set_all(uint16_t busi_type, uint8_t op, ip_any_t *ip,
                    lpm_user_result_t *data, dns_lpm_hlist_t *p)
{
    ipaddr_prefix_recalc(ip);
    // dns_lpm_set_debug(busi_type, op, ip, data);
    if (_dns_lpm_set_ip_policy(busi_type, op, ip, data, &p->head /*hash 入口*/,
                               0))
    {
        util_log_debug("failed");
        return -1;
    }

    // util_log_debug("successful");
    return 0;
}

bool dns_lpm_update(uint8_t op, uint16_t busi_type, lpm_user_result_t *data,
                    ip_any_t *ip, dns_lpm_hlist_t *hlist_new)
{
    if (op == 0 && hlist_new != NULL)
    {
        if (dns_lpm_set_all(busi_type, OP_ADD, ip, data, hlist_new) < 0)
        {
            return false;
        }
    }
    else
    {
        util_log_error("dns lpm load file invalid para. op: %u, hlist_new: %px", op,
                       hlist_new);
        return false;
    }

    return true;
}

int dns_lpm_table_addrs_group_add(dns_lpm_hlist_t *hlist_new, ip_any_t *ip,
                                  lpm_user_result_t *data)
{
    return dns_lpm_set_all(LPM_RESULT_TYPE_ID, OP_ADD, ip, data, hlist_new);
}

void dns_lpm_get_ip_policy_info(dns_lpm_policy_data_out_t *out,
                                dns_lpm_policy_data_t *in)
{
    if (out == NULL || in == NULL)
    {
        util_log_error("dns_lpm_get_ip_policy_info para=NULL");
        return;
    }

    if (out->user_result_cnt >= UTIL_LPM_MAX_MATCH)
    {
        util_log_error(
            "dns_lpm_get_ip_policy_info out->user_result_cnt >= MAX_USER_RESULT");
        return;
    }

    out->user_result[out->user_result_cnt] = in->user_result;
    out->user_result_cnt++;
}

void __dns_lpm_get_ip_policy(uint8_t is_ipv6, unsigned char *ip,
                             dns_lpm_node_t *dns_lpm_node,
                             dns_lpm_policy_data_out_t *dns_lpm_policy_data)
{
    dns_lpm_policy_data_t *current_ip_info = NULL;

    list_for_each_entry(current_ip_info,
                        is_ipv6 == IS_IPV4 ? &(dns_lpm_node->config_list_ipv4)
                                           : &(dns_lpm_node->config_list_ipv6),
                        list)
    {
        dns_lpm_get_ip_policy_info(dns_lpm_policy_data, current_ip_info);
    }
    return;
}

void _dns_lpm_get_ip_policy(uint8_t is_ipv6, unsigned char *ip,
                            dns_lpm_node_t *dns_lpm_node,
                            dns_lpm_policy_data_out_t *dns_lpm_policy_data,
                            uint8_t level)
{
    dns_lpm_node_t *tmp_hash_head = NULL;
    dns_lpm_node_t *hash_node = NULL;

    if (is_ipv6 == IS_IPV4)
    {
        if (level >= MAX_LEVEL_IPV4)
            return;
    }
    else
    {
        if (level >= MAX_LEVEL_IPV6)
            return;
    }

    hash_node = dns_lpm_node->dns_lpm_node;
    if (hash_node == NULL)
    { /*dns_lpm_node 表示上层到此的入口，dns_lpm_node
         表示本层具体的实体*/
        return;
    }

    tmp_hash_head = &(hash_node[ip[level]]);
    __dns_lpm_get_ip_policy(is_ipv6, ip, tmp_hash_head,
                            dns_lpm_policy_data); // 获取当前level 的配置
    _dns_lpm_get_ip_policy(
        is_ipv6, ip, tmp_hash_head, dns_lpm_policy_data,
        level +
            1); // 递归往深处level走 刷新ip_info  掩码越大 优先级越高 (如某一配置
                // 更深level有 则更新，如果没，则保持当前level的配置)

    return;
}

void dns_lpm_table_match(dns_lpm_hlist_t *p, uint8_t is_ipv6, uint8_t *ip,
                         dns_lpm_policy_data_out_t *dns_lpm_policy_data_out)
{
    int i = 0;
    _dns_lpm_get_ip_policy(is_ipv6, ip, &p->head /*hash 入口*/,
                           dns_lpm_policy_data_out, 0);

    util_log_debug("user_result_cnt:%d",
                   dns_lpm_policy_data_out->user_result_cnt);
    for (i = 0; i < dns_lpm_policy_data_out->user_result_cnt; i++)
    {
        util_log_debug("    result :%d, user_result_id:%ld", i,
                       dns_lpm_policy_data_out->user_result[i].user_result_id);
    }

    return;
}

static void dns_lpm_policy_print(dns_lpm_policy_data_t *data, ip_any_t *ip,
                                 uint8_t is_ipv6)
{
    char ip_str[64] = {0};

    if (data == NULL || ip == NULL)
    {
        util_log_error("dns_lpm_policy_print para=NULL");
        return;
    }

    if (is_ipv6 == IS_IPV4)
    {
        inet_ntop4((unsigned char *)&ip->ipv4, (unsigned char *)ip_str,
                   sizeof(ip_str));
    }
    else
    {
        inet_ntop6((unsigned char *)&ip->ipv6, (unsigned char *)ip_str,
                   sizeof(ip_str));
    }
    util_log_debug("dns_lpm_policy_print ip:%s, mask:%u, user_result_id:%ld",
                   ip_str, data->mask, data->user_result.user_result_id);
}

void dns_lpm_print(dns_lpm_node_t *dns_lpm_node, ip_any_t *ip_stack,
                   uint8_t level, ip_query_protocol_t *filter)
{
    uint16_t i, index;
    uint8_t extra = 0;
    bool is_default_ipv4_printed,
        is_default_ipv6_printed; // 用于避免mask=0的规则展开打印

    dns_lpm_node_t *tmp_hash_head = NULL;
    dns_lpm_node_t *hash_node = NULL;
    dns_lpm_policy_data_t *current_ip_info = NULL;

    if (level >= MAX_LEVEL_IPV6 || dns_lpm_node->dns_lpm_node == NULL)
    {
        return;
    }

    hash_node = dns_lpm_node->dns_lpm_node;
    if (hash_node == NULL)
    {
        return;
    }

    is_default_ipv4_printed = is_default_ipv6_printed = false;

    if (filter == NULL)
    {
        for (i = 0; i <= MAX_INDEX; i++)
        {
            ip_stack->ip_byte[level] = i;
            tmp_hash_head = &(hash_node[i]);

            // util_log_debug("dns_lpm_print level: %u, index: %u, hash head: %px",
            // level, i, hash_node);

            if (level < MAX_LEVEL_IPV4)
            {
                list_for_each_entry(current_ip_info, &tmp_hash_head->config_list_ipv4,
                                    list)
                {
                    if (current_ip_info->mask == 0 && is_default_ipv4_printed == true)
                    {
                        continue;
                    }
                    else if (current_ip_info->mask == 0 &&
                             is_default_ipv4_printed == false)
                    {
                        is_default_ipv4_printed = true;
                    }
                    dns_lpm_policy_print(current_ip_info, ip_stack, IS_IPV4);
                }
            }

            list_for_each_entry(current_ip_info, &tmp_hash_head->config_list_ipv6,
                                list)
            {
                if (current_ip_info->mask == 0 && is_default_ipv6_printed == true)
                {
                    continue;
                }
                else if (current_ip_info->mask == 0 &&
                         is_default_ipv6_printed == false)
                {
                    is_default_ipv6_printed = true;
                }
                dns_lpm_policy_print(current_ip_info, ip_stack, IS_IPV6);
            }

            if (tmp_hash_head->dns_lpm_node != NULL)
            {
                // util_log_debug("dns_lpm_print go deeper");
                dns_lpm_print(tmp_hash_head, ip_stack, level + 1, filter);
            }

            memset(ip_stack->ip_byte + level + 1, 0,
                   sizeof(ip_stack->ip_byte) - level - 1);
        }
    }
    else
    {
        ip_stack->ip_byte[level] = filter->ip_byte[level];
        tmp_hash_head = &(hash_node[filter->ip_byte[level]]);

        if ((level + 1) * 8 < filter->mask && tmp_hash_head->dns_lpm_node != NULL)
        {
            dns_lpm_print(tmp_hash_head, ip_stack, level + 1, filter);
        }
        else if ((level + 1) * 8 >= filter->mask)
        {
            extra = (~(0xff << ((level + 1) * 8 - filter->mask)) & 0xff);
            for (i = 0; i <= extra; i++)
            {
                index = filter->ip_byte[level] + i;
                if (index > MAX_INDEX)
                {
                    break;
                }

                ip_stack->ip_byte[level] = index;
                tmp_hash_head = &(hash_node[index]);
                list_for_each_entry(current_ip_info,
                                    filter->is_ipv6 == IS_IPV4
                                        ? &(tmp_hash_head->config_list_ipv4)
                                        : &(tmp_hash_head->config_list_ipv6),
                                    list)
                {
                    // 查询时要么为v4要么为v6, 所以仅使用v4的flag
                    if (current_ip_info->mask == filter->mask)
                    {
                        if (current_ip_info->mask == 0 && is_default_ipv4_printed == true)
                        {
                            continue;
                        }
                        else if (current_ip_info->mask == 0 &&
                                 is_default_ipv4_printed == false)
                        {
                            is_default_ipv4_printed = true;
                        }
                        dns_lpm_policy_print(current_ip_info, ip_stack, filter->is_ipv6);
                    }
                }
            }
        }
    }
}

void dns_lpm_free(dns_lpm_node_t *dns_lpm_node)
{
    int i = 0;
    struct list_head *pos = NULL, *n = NULL;
    dns_lpm_policy_data_t *dns_lpm_policy_data = NULL;

    if (dns_lpm_node == NULL)
    {
        return;
    }

    for (i = 0; i <= MAX_INDEX; i++)
    {
        if (dns_lpm_node[i].dns_lpm_node)
        {
            dns_lpm_free(dns_lpm_node[i].dns_lpm_node); // 递归释放
            dns_lpm_node[i].dns_lpm_node = NULL;
        }
        list_for_each_safe(pos, n, &dns_lpm_node[i].config_list_ipv4)
        {
            dns_lpm_policy_data = list_entry(pos, dns_lpm_policy_data_t, list);
            UTIL_FREE(dns_lpm_policy_data);
        }
        list_for_each_safe(pos, n, &dns_lpm_node[i].config_list_ipv6)
        {
            dns_lpm_policy_data = list_entry(pos, dns_lpm_policy_data_t, list);
            UTIL_FREE(dns_lpm_policy_data);
        }
    }
    UTIL_FREE(dns_lpm_node);
    dns_lpm_node = NULL;
    return;
}

dns_lpm_hlist_t *dns_lpm_table_create()
{
    dns_lpm_hlist_t *dns_lpm_hlist = NULL;
    if (dns_lpm_hlist == NULL)
    {
        dns_lpm_hlist = UTIL_MALLOC(sizeof(dns_lpm_hlist_t));
        memset(dns_lpm_hlist, 0, sizeof(dns_lpm_hlist_t));
    }
    else
    {
        goto FAIL;
    }
    dns_lpm_hlist->head.dns_lpm_node = NULL;
    return dns_lpm_hlist;
FAIL:
    if (dns_lpm_hlist != NULL)
    {
        UTIL_FREE(dns_lpm_hlist);
        dns_lpm_hlist = NULL;
    }
    return NULL;
}

void dns_lpm_table_free(dns_lpm_hlist_t *dns_lpm_hlist)
{
    if (dns_lpm_hlist != NULL)
    {
        if (dns_lpm_hlist->head.dns_lpm_node)
        {
            dns_lpm_free(dns_lpm_hlist->head.dns_lpm_node);
        }
        UTIL_FREE(dns_lpm_hlist);
        dns_lpm_hlist = NULL;
    }
    return;
}
