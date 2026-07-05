#include "backtrack.h"
#include "intent_log.h"
#include <string.h>
#include <stdlib.h>

/* 简化版 APP 场景判断：当前快照若有 APP 使用则视为该场景的候选
 * 实际项目应通过 SceneConfig 中的 appSceneMap 进行精确匹配 */
static int IsAppForScene(Snapshot *Snapshot, SceneRule *Rule, const char *sceneMap) {
    (void)Rule;
    (void)sceneMap;
    if (Snapshot->appUsageCount == 0) return 0;
    return 1;
}

/* BY_APP_FIRST_ACTIVE 策略：在 lookBackMs 窗口内寻找最早的 APP 匹配快照 */
static int BacktrackByAppFirstActive(Snapshot *SnapshotList, int SnapshotCount,
                                      int StartIdx, SceneRule *Rule, int *IntentIdx) {
    (void)SnapshotCount;
    long long windowStart = SnapshotList[StartIdx].snapshotTime - Rule->fieldView.lookBackMs;
    int found = -1;
    for (int i = StartIdx; i >= 0; i--) {
        if (SnapshotList[i].snapshotTime < windowStart) break;
        if (SnapshotList[i].appUsageCount > 0
            && IsAppForScene(&SnapshotList[i], Rule, Rule->sceneName)) {
            found = i;
            /* 首次匹配即最早，继续往前找首次 */
            for (int j = i - 1; j >= 0; j--) {
                if (SnapshotList[j].snapshotTime < windowStart) break;
                if (SnapshotList[j].appUsageCount > 0
                    && IsAppForScene(&SnapshotList[j], Rule, Rule->sceneName)) {
                    found = j;
                } else {
                    break;
                }
            }
            break;
        }
    }
    if (found < 0) return 1;
    *IntentIdx = found;
    return 0;
}

/* BY_STATE_CHANGE 策略：在 lookBackMs 窗口内寻找 commute 状态变化点 */
static int BacktrackByStateChange(Snapshot *SnapshotList, int SnapshotCount,
                                   int StartIdx, SceneRule *Rule, int *IntentIdx) {
    (void)SnapshotCount;
    long long windowStart = SnapshotList[StartIdx].snapshotTime - Rule->fieldView.lookBackMs;
    int found = -1;
    for (int i = StartIdx; i >= 0; i--) {
        if (SnapshotList[i].snapshotTime < windowStart) break;
        /* 查找 POI/通勤状态变化的快照（与前一个不同的快照） */
        if (i > 0 && SnapshotList[i].commutePeriod != SnapshotList[i - 1].commutePeriod) {
            found = i;
            break;
        }
    }
    if (found < 0) return 1;
    *IntentIdx = found;
    return 0;
}

/* SELF_AS_INTENT 策略：当前快照即为意图快照 */
static int BacktrackSelfAsIntent(Snapshot *SnapshotList, int StartIdx,
                                  int *IntentIdx) {
    *IntentIdx = StartIdx;
    (void)SnapshotList;
    return 0;
}

/* 主入口：根据策略分发到不同回溯实现 */
int BacktrackIntentSnapshot(Snapshot *SnapshotList, int SnapshotCount,
                            int StartIdx, SceneRule *Rule,
                            BacktrackStrategy Strategy, int *IntentIdx) {
    if (SnapshotList == NULL || Rule == NULL || IntentIdx == NULL) {
        return INTENT_FILTER_ERR_PARAM;
    }
    if (StartIdx < 0 || StartIdx >= SnapshotCount) {
        return INTENT_FILTER_ERR_PARAM;
    }
    switch (Strategy) {
        case BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE:
            return BacktrackByAppFirstActive(SnapshotList, SnapshotCount,
                                              StartIdx, Rule, IntentIdx);
        case BACKTRACK_STRATEGY_BY_APP_LAST_ACTIVE:
            /* 当前实现同 FIRST_ACTIVE，后续可扩展 */
            return BacktrackByAppFirstActive(SnapshotList, SnapshotCount,
                                              StartIdx, Rule, IntentIdx);
        case BACKTRACK_STRATEGY_BY_STATE_CHANGE:
            return BacktrackByStateChange(SnapshotList, SnapshotCount,
                                           StartIdx, Rule, IntentIdx);
        case BACKTRACK_STRATEGY_SELF_AS_INTENT:
            return BacktrackSelfAsIntent(SnapshotList, StartIdx, IntentIdx);
        default:
            LOG_ERROR("Unknown backtrack strategy: %d", Strategy);
            return INTENT_FILTER_ERR_PARAM;
    }
}