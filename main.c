#include <stdio.h>
#include <string.h>

#include "test.h"

int main() {
    // char dname[100] = {0x3, 'w', 'w', 'w', 0x4, 'b', 'b', 'b', 'b', 0x2,  'c',
    // 'c', }; char dname[100] = {0x1, 'a', 0x2, 'b', 'b', 0x0}; char *d1 =
    // "www.baidu.com";
    char *d1       = "www.aa.bb.cc.dd.ee.ff";
    char  d2[1000] = {};
    char *dname    = d2;

    // trans_to_dname_format(d1, d2);

    // test_seg(dname);

    test_list();

    return 0;
}
