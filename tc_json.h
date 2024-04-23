#ifndef _TC_JSON_H_

#define _TC_JSON_H_

#define RR_NUM_MAX (32)


//{"bt":"dns-force", "sbt":"dn_limit", "qtype":"aaaa", "rr":["1.1.1.1", "2.2.2.2" ] }
typedef struct nap_cmd
{
    uint8_t rr_num;
    char *bt;
    char *sbt;
    char *qtype;
    char *rr[RR_NUM_MAX];
} nap_cmd_t;


int tc_json_test(int argc, char* argv[]);











#endif