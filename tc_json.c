#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <yyjson.h>

#include "comm.h"
#include "cJSON.h"

extern int test_rust_json_deserialize(char *json_str);

long long g_read_len;
long long g_write_len;

typedef void (*json_test_cb)(char *json_str, int len);

void test_yyjson(char *json_str, int len)
{
    printf("Try use yyjson test....\n");
    yyjson_doc *doc = yyjson_read(json_str, strlen(json_str), 0);
    yyjson_val *root = yyjson_doc_get_root(doc);
    yyjson_val *name = yyjson_obj_get(root, "bt");
    printf("name: %s\n", yyjson_get_str(name));
    printf("name length:%d\n", (int)yyjson_get_len(name));
    yyjson_doc_free(doc);
    while (1)
    {
        doc = yyjson_read(json_str, strlen(json_str), 0);
        yyjson_doc_free(doc); // printf("Cjson:%s \n", cJSON_Print(json_obj));
        g_read_len += len;
    };
}

void test_cjson(char *json_str, int len)
{
    printf("Try use cjson test....\n");
    cJSON *json_obj = NULL;
    while (1)
    {
        json_obj = cJSON_Parse(json_str);

        cJSON_Delete(json_obj);
        // printf("Cjson:%s \n", cJSON_Print(json_obj));
        g_read_len += len;
    };
}

void test_rs_serde_json(char *json_str, int len)
{
    printf("Try use rs serde json test....\n");
    while (1)
    {
        test_rust_json_deserialize(json_str);
        g_read_len += len;
    };
}

void test_rust_cjson(json_test_cb test_func)
{
#define MAX_JSON_LINE_LEN (1024)
    char *path = "./nap_cmd.json";
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        printf("无法打开文件\n");
        return;
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
    return;
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
        printf("read_Bps:%llu\twrite_Bps:%llu\n", (new_read_cnt - old_read_cnt) / sec,
               (new_write_cnt - old_write_cnt) / sec);
        old_read_cnt = new_read_cnt;
        old_write_cnt = new_write_cnt;
        old_time = new_time;
    }
}

bool create_time_thread()
{
    uint32_t ret = 0;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, get_pps_loop, NULL);

    if (ret < 0)
    {
        printf("创建获取实时时间线程失败\n");
        return false;
    }

    pthread_detach(pid);
    return true;
}



struct json_test {
    char *lib_name;
    json_test_cb test_func;

} g_json_types[] = {
	{"cjson", test_cjson},
    {"yyjson", test_yyjson},
    {"serdejson", test_rs_serde_json},
} ;

int tc_json_test(int argc, char *argv[])
{
    json_test_cb test_func = test_cjson;
    
    if (argc == 2)
    {
        int i = 0;
        for(i=0; i<ARRAY_LENGTH(g_json_types); i++){
            if(strcmp(argv[1], g_json_types[i].lib_name) == 0)
            {
                test_func = g_json_types[i].test_func;
            }
        }
    }
    create_time_thread();
    test_rust_cjson(test_func);

    return 0;
}
