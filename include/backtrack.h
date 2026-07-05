#ifndef BACKTRACK_H
#define BACKTRACK_H

#include "intent_types.h"

/**
 * 回溯查找意图发生时刻快照
 * @param SnapshotList    快照数组（按时间升序）
 * @param SnapshotCount   快照数量
 * @param StartIdx        当前触发快照索引
 * @param Rule            命中的场景规则
 * @param Strategy        回溯策略
 * @param IntentIdx       输出意图快照索引
 * @return 0 找到, 1 未找到丢弃, 负数异常
 */
int BacktrackIntentSnapshot(Snapshot *SnapshotList, int SnapshotCount,
                            int StartIdx, SceneRule *Rule,
                            BacktrackStrategy Strategy, int *IntentIdx);

#endif /* BACKTRACK_H */