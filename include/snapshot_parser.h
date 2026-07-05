#ifndef SNAPSHOT_PARSER_H
#define SNAPSHOT_PARSER_H

#include "intent_types.h"

/**
 * 解析 JSON 字符串为 Snapshot 结构体
 * @param JsonString 输入 JSON 字符串
 * @param Snapshot   输出快照（调用方分配）
 * @return INTENT_FILTER_OK 成功, 负数失败
 */
int SnapshotParseFromJson(const char *JsonString, Snapshot *Snapshot);

/**
 * 释放 Snapshot 内部所有动态分配的内存
 * @param Snapshot 要释放的快照
 */
void SnapshotDestroy(Snapshot *Snapshot);

/**
 * 从文件加载并解析 Snapshot
 * @param FilePath JSON 文件路径
 * @param Snapshot 输出快照
 * @return INTENT_FILTER_OK 成功, 负数失败
 */
int SnapshotParseFromFile(const char *FilePath, Snapshot *Snapshot);

#endif /* SNAPSHOT_PARSER_H */