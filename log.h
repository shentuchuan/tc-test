#ifndef _TEST_LOG_H
#define _TEST_LOG_H


typedef enum {
	LOG_DEBUG = 0,
	LOG_INFO,
	LOG_NOTICE,
	LOG_WARN,
	LOG_ERR,
	LOG_MAX
}LOG_LEVEL_E;

#define _DEBUG_LOG
#ifdef _DEBUG_LOG
extern int g_log_level;
extern int g_usr_log_def;
#define TLog(fmt, args...) \
do{\
	if(g_usr_log_def >= g_log_level) {\
		printf(fmt, ##args);\
	}\
}while(0)


#define TLogSTD(level, fmt, args...) \
do{\
	if(level >= g_log_level) {\
		printf(fmt, ##args);\
	}\
}while(0)

#else

#define TLog(fmt, args...) 

#define TLogSTD(level, fmt, args...) 
#endif




#endif

