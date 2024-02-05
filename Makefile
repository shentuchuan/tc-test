APP=tc_test


ifndef K_VER
	K_VER = $(shell uname -r)
endif

_K_VER = $(shell uname -r | awk -F '-' '{print $$1}')

KERNEL_SRC = /lib/modules/$(K_VER)/build
KERNEL_MOD = /lib/modules/$(K_VER)/kernel
KERNEL_DEP = ${BUILD_PATH}/lib/modules/$(K_VER)/modules.dep

ARCH = $(shell uname -m)
SCRIPT = $(KERNEL_SRC)/scripts


ifeq (3.10.0, $(_K_VER))
DEF_OPT = -DKERNEL_3_10
endif

ifeq (3.4.51, $(_K_VER))
DEF_OPT = -DKERNEL_3_4
endif

ifeq (4.18.10, $(_K_VER))
DEF_OPT = -DKERNEL_4_18
endif

ifeq (5.10.0, $(_K_VER))
DEF_OPT = -DKERNEL_5_10
endif

ifeq (aarch64, $(ARCH))
DEF_OPT += -DAARCH64
endif

DEF_OPT += -D__KERNEL__ -DMODULE -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 \
	  -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCC_HAVE_ASM_GOTO -Wdeclaration-after-statement

EXTRA_CFLAGS += $(DEF_OPT) 

# 获取当前目录下的所有源文件
SRCS := $(wildcard *.c)
# 将源文件列表转换为对应的目标文件列表
OBJS := $(SRCS:.c=.o)
# 定义编译器和编译选项
CC := gcc

CFLAGS := -Wall -g

#增加头文件路径
#CFLAGS += -I/usr/src/kernels/5.10.0-136.37.0.113.ky10.x86_64/include/ -I/usr/src/kernels/5.10.0-136.37.0.113.ky10.x86_64/arch/x86/include/generated
#CFLAGS += -I/usr/src/kernels//5.10.0-136.37.0.113.ky10.x86_64/arch/x86/include/ 

#关闭未使用变量告警
#CFLAGS += -Wno-unused-but-set-variable


# 默认目标，即执行make命令时的默认行为
all: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(APP)

# 编译每个源文件为对应的目标文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理编译生成的文件
clean:
	rm -f $(OBJS) $(APP)
