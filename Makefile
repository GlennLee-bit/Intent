# Makefile for intent filter
CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -Werror -O2 -g -Iinclude -Ithird_party/cJSON
AR := ar
ARFLAGS := rcs

SRC_DIR := src
INC_DIR := include
TEST_DIR := test
TP_DIR := third_party/cJSON
BUILD_DIR := build

LIB_NAME := libintent.a
LIB_PATH := $(BUILD_DIR)/$(LIB_NAME)

# 业务源文件
LIB_SRCS := $(wildcard $(SRC_DIR)/*.c)
LIB_OBJS := $(LIB_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# cJSON 源文件
CJSON_OBJ := $(BUILD_DIR)/cJSON.o

# 测试源文件 (排除 test_helpers.c，它只提供公共代码)
TEST_SRCS := $(filter-out $(TEST_DIR)/test_helpers.c, $(wildcard $(TEST_DIR)/test_*.c))
TEST_RUNNERS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_DIR)/%)

TEST_HELPERS_SRC := $(TEST_DIR)/test_helpers.c

.PHONY: all clean test test-all integration help

# 默认目标: 构建库 + 所有测试 runner
all: $(LIB_PATH) $(TEST_RUNNERS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 业务 .o 编译规则
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(CJSON_OBJ): $(TP_DIR)/cJSON.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_PATH): $(LIB_OBJS) $(CJSON_OBJ)
	$(AR) $(ARFLAGS) $@ $^

# 测试编译: 每个 test_X.c 编译为独立的 test_X runner
# 模式: build/test_X 依赖于 build/test_X.o + 库 + test_helpers.o
$(BUILD_DIR)/test_%: $(BUILD_DIR)/test_%.o $(LIB_PATH) $(BUILD_DIR)/test_helpers.o
	$(CC) $(CFLAGS) -Itest $^ -o $@

# 测试 .o 编译规则 (test_helpers 单独编译为可重用的 .o)
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Itest -c $< -o $@

# 运行所有测试
test test-all: $(TEST_RUNNERS)
	@set -e; \
	for runner in $(TEST_RUNNERS); do \
		echo "=== Running $$runner ==="; \
		$$runner || exit 1; \
	done

# 单独运行某个测试 (示例: make integration)
integration: $(BUILD_DIR)/test_integration
	./$(BUILD_DIR)/test_integration

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Available targets:"
	@echo "  all         - Build library + all test runners"
	@echo "  test        - Build and run all tests"
	@echo "  integration - Run integration test only"
	@echo "  clean       - Remove build artifacts"