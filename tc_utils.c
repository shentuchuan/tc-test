#include <stdio.h>
#include <time.h>
#include <sys/syscall.h>
#include <syscall.h>
#include <inttypes.h>

#include "tc_utils.h"

//#define PDEBUG printf

#define PDEBUG 



struct {
    TC_ATOMIC_DECLARE(uint64_t, counter);
	const char *desc;
}g_tick_cnt[TC_CNT_MAX];

int tc_tick_cnt_add(TC_CNT_E type, uint64_t start, uint64_t end)
{
	if((type < 0) || (type >= TC_CNT_MAX)) {
		return -1;
	}

	TC_ATOMIC_ADD(g_tick_cnt[type].counter, end-start);
	return 0;
}


int tc_tick_cnt_dump()
{
	PDEBUG("Ticks dump:\n");
	int i = 0;
	for(i=0; i<TC_CNT_MAX; i++) {	
		if(g_tick_cnt[i].desc) {
			PDEBUG("%30s : %d\n", g_tick_cnt[i].desc, TC_ATOMIC_GET(g_tick_cnt[i].counter));
		}	
		else {
			PDEBUG("%30d : %d\n", i, TC_ATOMIC_GET(g_tick_cnt[i].counter));
		}
	}
}


int tc_tick_cnt_file_dump(FILE *fp)
{
#define FPDEBUG(...)   fprintf(fp, __VA_ARGS__)

	FPDEBUG("Ticks dump:\n");
	int i = 0;
	for(i=0; i<TC_CNT_MAX; i++) {	
		if(g_tick_cnt[i].desc) {
			FPDEBUG("%30s : %20llu\n", g_tick_cnt[i].desc, TC_ATOMIC_GET(g_tick_cnt[i].counter));
		}	
		else {
			FPDEBUG("%30d : %20llu\n", i, TC_ATOMIC_GET(g_tick_cnt[i].counter));
		}
	}
	return 0;
}


int tc_tick_cnt_type_set(TC_CNT_E type, const char *desc)
{
	if((type < 0) || (type >= TC_CNT_MAX)) {
		return -1;
	}
	TC_ATOMIC_INIT(g_tick_cnt[type].counter);
	g_tick_cnt[type].desc = desc;
	return 0;
}

int tc_tick_cnt_init()
{
	int i = 0;
	tc_tick_cnt_type_set(TC_CNT_DEQ, "xforward-send-deq");
	tc_tick_cnt_type_set(TC_CNT_DO_FWD, "xforward-send-fwd");
	tc_tick_cnt_type_set(TC_CNT_ENQ, "xforward-send-enq");
	tc_tick_cnt_type_set(TC_CNT_SRV_NEW, "xforward-send-srv");
	tc_tick_cnt_type_set(TC_CNT_CLI_NEW, "xforward-send-cli");
	tc_tick_cnt_type_set(TC_CNT_SOCKET_SEND, "xforward-send-udp");
	tc_tick_cnt_type_set(TC_CNT_MEM_FREE, "xforward-send-mem-free");
	
	return 0;
}


/**
 * Get the current number of ticks from the CPU.
 *
 * \todo We'll have to deal with removing ticks from the extra cpuids in between
 *       2 calls.
 */
uint64_t cpu_ticks_get(void)
{
    uint64_t val;
#if defined(__GNUC__) && (defined(__x86_64) || defined(_X86_64_) || defined(ia_64) || defined(__i386__))
#if defined(__x86_64) || defined(_X86_64_) || defined(ia_64)
	PDEBUG("------------------------------1\n");
    __asm__ __volatile__ (
    "xorl %%eax,%%eax\n\t"
    "cpuid\n\t"
    ::: "%rax", "%rbx", "%rcx", "%rdx");
#else
	PDEBUG("------------------------------2\n");
    __asm__ __volatile__ (
    "xorl %%eax,%%eax\n\t"
    "pushl %%ebx\n\t"
    "cpuid\n\t"
    "popl %%ebx\n\t"
    ::: "%eax", "%ecx", "%edx");
#endif
	PDEBUG("------------------------------3\n");
    uint32_t a, d;
    __asm__ __volatile__ ("rdtsc" : "=a" (a), "=d" (d));
    val = ((uint64_t)a) | (((uint64_t)d) << 32);
#if defined(__x86_64) || defined(_X86_64_) || defined(ia_64)
	PDEBUG("------------------------------4\n");
    __asm__ __volatile__ (
    "xorl %%eax,%%eax\n\t"
    "cpuid\n\t"
    ::: "%rax", "%rbx", "%rcx", "%rdx");
#else
	PDEBUG("------------------------------5\n");
    __asm__ __volatile__ (
    "xorl %%eax,%%eax\n\t"
    "pushl %%ebx\n\t"
    "cpuid\n\t"
    "popl %%ebx\n\t"
    ::: "%eax", "%ecx", "%edx");
#endif

#else /* #if defined(__GNU__) */
//#warning Using inferior version of UtilCpuGetTicks
	PDEBUG("------------------------------5\n");
    struct timeval now;
    gettimeofday(&now, NULL);
    val = (now.tv_sec * 1000000) + now.tv_usec;
#endif
	return val;
}



void * file_read(char *path) {
#define MAX_LINE_LENGTH (1024)	
	if(NULL == path) {
		return NULL;
	}

    FILE *file = fopen(path, "r");  
    if (file == NULL) {
        printf("无法打开文件\n");
        return NULL;
    }
    
    char line[MAX_LINE_LENGTH];  // 用于存储每行内容的缓冲区
    
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        printf("%s", line);  // 打印每行内容
    }

    fclose(file);  // 关闭文件

    return 0;
}
