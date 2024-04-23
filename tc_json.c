#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
//#include <yyjson.h>

#include "comm.h"
#include "test.h"
#include "tc_json.h"
#include "cJSON.h"

extern int test_rust_json_read(char *json_str);
extern int test_rust_json_write(nap_cmd_t *nap_cmd);

long long g_read_len;
long long g_write_len;

typedef void (*json_test_cb)(char *json_str, int len);

void nap_cmd_dump(nap_cmd_t *nap_cmd)
{
    printf("Nap cmd :\n");
    printf("    bt : %s\n", nap_cmd->bt);
    printf("    sbt : %s\n", nap_cmd->sbt);
    printf("    qtype : %s\n", nap_cmd->qtype);
    int rr_num = 0;
    for (rr_num = 0; rr_num < nap_cmd->rr_num; rr_num++)
    {
        printf("    rr : %s\n", nap_cmd->rr[rr_num]);
    }
}

#if 0
void yyjson_to_nap_cmd(nap_cmd_t *nap_cmd, yyjson_doc *doc)
{
    yyjson_val *root = yyjson_doc_get_root(doc);

    yyjson_val *name = yyjson_obj_get(root, "bt");
    nap_cmd->bt = yyjson_get_str(name);

    name = yyjson_obj_get(root, "sbt");
    nap_cmd->sbt = yyjson_get_str(name);

    name = yyjson_obj_get(root, "qtype");
    nap_cmd->qtype = yyjson_get_str(name);

    name = yyjson_obj_get(root, "rr");
    nap_cmd->rr_num = yyjson_arr_size(name);

    int i = 0;
    yyjson_val *mem = NULL;
    for (i = 0; i < nap_cmd->rr_num; i++)
    {
        mem = yyjson_arr_get(name, i);
        nap_cmd->rr[i] = yyjson_get_str(mem);
    }
    // nap_cmd_dump(nap_cmd);
}

void test_yyjson_read(char *json_str, int len)
{
    nap_cmd_t nap_cmd = {};
    printf("Try use yyjson test read ....\n");
    yyjson_doc *doc = yyjson_read(json_str, strlen(json_str), 0);
    yyjson_to_nap_cmd(&nap_cmd, doc);

    yyjson_doc_free(doc);
    while (1)
    {
        doc = yyjson_read(json_str, strlen(json_str), 0);
        // yyjson_to_nap_cmd(&nap_cmd, doc);
        yyjson_doc_free(doc); // printf("Cjson:%s \n", cJSON_Print(json_obj));
        g_read_len += 1;
    };
}

void test_yyjson_write(char *json_str, int len)
{
    nap_cmd_t *nap_cmd = (nap_cmd_t *)json_str;
    printf("Try use yyjson test write ....\n");

    while (1)
    {
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);
        yyjson_mut_obj_add_str(doc, root, "bt", nap_cmd->bt);
        yyjson_mut_obj_add_str(doc, root, "sbt", nap_cmd->sbt);
        yyjson_mut_obj_add_str(doc, root, "qtype", nap_cmd->qtype);
        yyjson_mut_val *arr = yyjson_mut_arr_with_str(doc, nap_cmd->rr, nap_cmd->rr_num);
        yyjson_mut_obj_add_val(doc, root, "rr", arr);

        // printf("create json: %s\n", yyjson_mut_write(doc, 0, &len));
        yyjson_mut_doc_free(doc);
        g_write_len += 1;
    };
}

void test_cjson_read(char *json_str, int len)
{
    printf("Try use cjson test read ....\n");
    cJSON *json_obj = NULL;
    while (1)
    {
        json_obj = cJSON_Parse(json_str);

        cJSON_Delete(json_obj);
        // printf("Cjson:%s \n", cJSON_Print(json_obj));
        g_read_len += 1;
    };
}

void test_cjson_write(char *json_str, int len)
{
    nap_cmd_t *nap_cmd = (nap_cmd_t *)json_str;
    printf("Try use cjson test write ....\n");
/*     cJSON *cjson_obj = cJSON_CreateObject();
    cJSON *cjson_bt = cJSON_CreateString(nap_cmd->bt);
    cJSON_AddItemToObject(cjson_obj, "bt", cjson_bt);

    cJSON *cjson_sbt = cJSON_CreateString(nap_cmd->sbt);
    cJSON_AddItemToObject(cjson_obj, "sbt", cjson_sbt);

    cJSON *cjson_qtype = cJSON_CreateString(nap_cmd->qtype);
    cJSON_AddItemToObject(cjson_obj, "qtype", cjson_qtype);

    cJSON *cjson_rr = cJSON_CreateStringArray(nap_cmd->rr, nap_cmd->rr_num);
    cJSON_AddItemToObject(cjson_obj, "rr", cjson_rr);
    printf(" Write Cjson:%s \n", cJSON_Print(cjson_obj));
    cJSON_Delete(cjson_obj);
 */
    while (1)
    {
        cJSON *cjson_obj = cJSON_CreateObject();
        cJSON *cjson_bt = cJSON_CreateString(nap_cmd->bt);
        cJSON_AddItemToObject(cjson_obj, "bt", cjson_bt);

        cJSON *cjson_sbt = cJSON_CreateString(nap_cmd->sbt);
        cJSON_AddItemToObject(cjson_obj, "sbt", cjson_sbt);

        cJSON *cjson_qtype = cJSON_CreateString(nap_cmd->qtype);
        cJSON_AddItemToObject(cjson_obj, "qtype", cjson_qtype);

        cJSON *cjson_rr = cJSON_CreateStringArray(nap_cmd->rr, nap_cmd->rr_num);
        cJSON_AddItemToObject(cjson_obj, "rr", cjson_rr);
        cJSON_Delete(cjson_obj);
        // printf("Cjson:%s \n", cJSON_Print(json_obj));
        g_write_len += 1;
    };
}

void test_serdejson_read(char *json_str, int len)
{
    printf("Try use rs serde json read ....\n");
    while (1)
    {
        test_rust_json_read(json_str);
        g_read_len += 1;
    };
}

void test_serdejson_write(char *json_str, int len)
{
    nap_cmd_t *nap_cmd = (nap_cmd_t *)json_str;
    printf("Try use rs serde json write ....\n");
    // nap_cmd_dump(nap_cmd);
    // test_rust_json_write(nap_cmd, 2);
    while (1)
    {
        test_rust_json_write(nap_cmd);
        g_write_len += 1;
    };
}

void *read_rust_cjson_test(void *ptest_func)
{
    json_test_cb test_func = (json_test_cb)ptest_func;

#define MAX_JSON_LINE_LEN (1024)
    char *path = "./nap_cmd.json";
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        printf("无法打开文件\n");
        return NULL;
    }

    char line[MAX_JSON_LINE_LEN]; // 用于存储每行内容的缓冲区

    while (fgets(line, MAX_JSON_LINE_LEN, file) != NULL)
    {

        printf("Get line: %s\n", line); // 打印每行内容
        int len = strlen(line);
        // test_cjson(line, len);
        test_func(line, len);
        // test_rs_serde_json(line, len);
    }

    fclose(file); // 关闭文件
    return NULL;
}

void *write_rust_cjson_test(void *ptest_func)
{
    json_test_cb test_func = (json_test_cb)ptest_func;

#define MAX_JSON_LINE_LEN (1024)
    char *path = "./nap_cmd.json";
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        printf("无法打开文件\n");
        return NULL;
    }

    char line[MAX_JSON_LINE_LEN]; // 用于存储每行内容的缓冲区
    nap_cmd_t nap_cmd = {};

    while (fgets(line, MAX_JSON_LINE_LEN, file) != NULL)
    {
        break;
    }
    printf("Get line: %s\n", line); // 打印每行内容
    int len = strlen(line);
    yyjson_doc *doc = yyjson_read(line, strlen(line), 0);
    yyjson_val *root = yyjson_doc_get_root(doc);
    yyjson_val *name = yyjson_obj_get(root, "bt");
    yyjson_to_nap_cmd(&nap_cmd, doc);

    // nap_cmd_dump(&nap_cmd);
    test_func((char *)&nap_cmd, 0);

    yyjson_doc_free(doc);
    fclose(file); // 关闭文件
    return NULL;
}

void *get_pps_loop(void *arg)
{
    time_t old_time;
    time_t new_time;
    uint64_t new_read_cnt = 0;
    uint64_t old_read_cnt = 0;

    uint64_t new_write_cnt = 0;
    uint64_t old_write_cnt = 0;

    sleep(2);
    old_read_cnt = g_read_len;
    old_write_cnt = g_write_len;
    old_time = time(NULL);

    for (;;)
    {
        sleep(2);
        new_time = time(NULL);
        new_read_cnt = g_read_len;
        new_write_cnt = g_write_len;
        long long unsigned sec = ((uint64_t)(new_time - old_time));
        printf("read_lps:%llu\twrite_lps:%llu\n", (new_read_cnt - old_read_cnt) / sec,
               (new_write_cnt - old_write_cnt) / sec);
        old_read_cnt = new_read_cnt;
        old_write_cnt = new_write_cnt;
        old_time = new_time;
    }
}

bool create_time_thread(pthread_t *pid)
{
    uint32_t ret = 0;

    ret = pthread_create(pid, NULL, get_pps_loop, NULL);

    if (ret < 0)
    {
        printf("创建获取实时时间线程失败\n");
        return false;
    }

    cpu_affinity_set(*pid, 6);

    // pthread_detach(*pid);
    return true;
}

bool read_json_thread_create(json_test_cb read_test_func)
{
    uint32_t ret = 0;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, read_rust_cjson_test, (void *)read_test_func);

    if (ret < 0)
    {
        printf("创建获取read线程失败\n");
        return false;
    }
    cpu_affinity_set(pid, 28);
    pthread_detach(pid);
    return true;
}

bool write_json_thread_create(json_test_cb write_test_func)
{
    uint32_t ret = 0;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, write_rust_cjson_test, (void *)write_test_func);

    if (ret < 0)
    {
        printf("创建获取write线程失败\n");
        return false;
    }
    cpu_affinity_set(pid, 29);
    pthread_detach(pid);
    return true;
}

struct json_test
{
    char *lib_name;
    json_test_cb read_func;
    json_test_cb write_func;
} g_json_types[] = {
    {"cjson", test_cjson_read, test_cjson_write},
    {"yyjson", test_yyjson_read, test_yyjson_write},
    {"serdejson", test_serdejson_read, test_serdejson_write},
};

int tc_json_test(int argc, char *argv[])
{
    json_test_cb read_test_func = test_serdejson_read;
    json_test_cb write_test_func = test_serdejson_write;

    if (argc == 2)
    {
        int i = 0;
        for (i = 0; i < ARRAY_LENGTH(g_json_types); i++)
        {
            if (strcmp(argv[1], g_json_types[i].lib_name) == 0)
            {
                read_test_func = g_json_types[i].read_func;
                write_test_func = g_json_types[i].write_func;
            }
        }
    }
    pthread_t pid;
    create_time_thread(&pid);
    read_json_thread_create(read_test_func);
    write_json_thread_create(write_test_func);

    pthread_join(pid, NULL);
    return 0;
}
#endif 