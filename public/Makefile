# 基础配置
CC = gcc
CFLAGS = -fPIC -Wall -O2 -g  # 必须包含 -fPIC
LDFLAGS = -shared          # 动态库链接选项

# 路径配置
CUR_PATH := $(shell pwd)
SRC_DIR = $(CUR_PATH)
BUILD_DIR = $(CUR_PATH)/build
INC_DIR = $(CUR_PATH)

# 源文件处理
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TARGET = libpublic.so      # 动态库命名

# 包含路径 (根据实际情况调整)
INCLUDES = -I$(INC_DIR) 
INCS += $(INCLUDES) 
# 库依赖 (示例，根据实际需要调整)
LIBS = 
#LIBS = -lcjson 
       
.PHONY: all clean install

all: $(BUILD_DIR) $(TARGET) test_build

$(BUILD_DIR):
	mkdir -p $@

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

###############################################################################
# 测试编译
TEST_DIR     = test
TEST_SRCS    = $(wildcard $(TEST_DIR)/test_*.c)
TEST_TARGETS = $(patsubst $(TEST_DIR)/%.c, $(TEST_BUILD)/%, $(TEST_SRCS))
TEST_BUILD   = $(BUILD_DIR)/test

# 添加测试构建目录创建规则
test_build:
	@mkdir -p $(TEST_BUILD)

# 添加测试编译规则（追加在现有规则后）
$(TEST_BUILD)/%: $(TEST_DIR)/%.c $(TARGET)
	@echo "正在编译测试: $<"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(TARGET) $(LIBS)
	@echo "已生成测试程序: $@"

test: $(TEST_TARGETS)
	@echo "\n=== 所有测试程序 ==="
	@ls -l $(TEST_BUILD) | awk 'NR>1 {printf "• %s\n", $$9}'

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_BUILD)

install:
	cp $(TARGET) /usr/local/lib
	ldconfig

run: test
	@echo "\n=== 执行所有测试 ==="
	@for t in $(TEST_TARGETS); do \
		echo "\n▶ 运行 $$t"; \
		$$t || exit 1; \
	done
	@echo "\n✅ 所有测试通过"