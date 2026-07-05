#ifndef INTENT_FILTER_H
#define INTENT_FILTER_H

#include "intent_types.h"

#define MAX_INTENT_CANDIDATES 32

/**
 * 初始化意图过滤器（加载场景配置）
 * @param Config 已加载的场景配置
 * @return INTENT_FILTER_OK 成功
 */
int IntentFilterInit(SceneConfig *Config);

/**
 * 处理一个用户的快照批次
 * @param SnapshotList    快照数组（已按时间升序）
 * @param SnapshotCount   快照数量
 * @param Result          输出结果（调用方预分配，candidateList 容量 >= MAX_INTENT_CANDIDATES）
 * @return INTENT_FILTER_OK 成功, 负数失败
 */
int IntentFilterProcess(Snapshot *SnapshotList, int SnapshotCount,
                        IntentResult *Result);

/**
 * 释放 IntentResult 内部动态分配的内存
 */
void IntentResultDestroy(IntentResult *Result);

#endif /* INTENT_FILTER_H */