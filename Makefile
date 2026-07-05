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

# 测试源文件
TEST_SRCS := $(wildcard $(TEST_DIR)/test_*.c)
TEST_OBJS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_DIR)/%.o)

TEST_RUNNER := $(BUILD_DIR)/test_runner
TEST_HELPERS_SRC := $(TEST_DIR)/test_helpers.c

.PHONY: all clean test

all: $(LIB_PATH) $(TEST_RUNNER)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(CJSON_OBJ): $(TP_DIR)/cJSON.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_PATH): $(LIB_OBJS) $(CJSON_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS -Itest) -c $< -o $@

$(TEST_RUNNER): $(TEST_OBJS) $(LIB_PATH) $(TEST_HELPERS_SRC)
	$(CC) $(CFLAGS) -Itest $(TEST_OBJS) $(LIB_PATH) $(TEST_HELPERS_SRC) -o $@

test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

clean:
	rm -rf $(BUILD_DIR)