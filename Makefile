APP=tc_test

#静态库
export LIBRARY_PATH += ./tc_rust/target/release/
#动态库
export LD_LIBRARY_PATH += ./

EXTRA_CFLAGS += $(DEF_OPT) 

# 获取当前目录下的所有源文件
SRCS := $(wildcard *.c)
# 将源文件列表转换为对应的目标文件列表
OBJS := $(SRCS:.c=.o)
# 定义编译器和编译选项
CC := gcc

CFLAGS := -Wall -g -O2
CFLAGS += -I/usr/local/include/
CFLAGS += -lpcap
CFLAGS += -lpthread -ldl 

#需要放在最后链接库
LAST_LIBS += -ltc_rust -lyyjson

#增加头文件路径
#CFLAGS += -I/usr/src/kernels/5.10.0-136.37.0.113.ky10.x86_64/include/ -I/usr/src/kernels/5.10.0-136.37.0.113.ky10.x86_64/arch/x86/include/generated
#CFLAGS += -I/usr/src/kernels//5.10.0-136.37.0.113.ky10.x86_64/arch/x86/include/ 


#关闭未使用变量告警
#CFLAGS += -Wno-unused-but-set-variable


# 默认目标，即执行make命令时的默认行为
all: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(APP) ${LAST_LIBS}

# 编译每个源文件为对应的目标文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理编译生成的文件
clean:
	rm -f $(OBJS) $(APP)
