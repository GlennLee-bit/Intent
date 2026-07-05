#include "intent_filter.h"
#include "scene_rule.h"
#include "backtrack.h"
#include "evidence.h"
#include "caption_fill.h"
#include "intent_log.h"
#include <string.h>
#include <stdlib.h>

/* 全局场景配置（Init 时设置，Process 时只读） */
static SceneConfig *gConfig = NULL;

/* 初始化：保存场景配置指针，运行时不可变 */
int IntentFilterInit(SceneConfig *Config) {
    if (Config == NULL) {
        LOG_ERROR("IntentFilterInit: NULL config");
        return INTENT_FILTER_ERR_PARAM;
    }
    gConfig = Config;
    LOG_INFO("IntentFilter initialized with %d scenes", Config->sceneRuleCount);
    return INTENT_FILTER_OK;
}

/* 处理单个快照：匹配规则 -> 回溯意图 -> 收集证据 -> 填充 caption -> 加入结果 */
static int ProcessOneSnapshot(Snapshot *SnapshotList, int SnapshotCount,
                              int Idx, IntentResult *Result) {
    SceneRule matched[MAX_MATCHED_RULES];
    int matchedCount = 0;
    int ret = MatchSceneRules(&SnapshotList[Idx], gConfig, matched, &matchedCount);
    if (ret != INTENT_FILTER_OK) return ret;
    int hasAppMatch = 0;
    for (int i = 0; i < matchedCount; i++) {
        /* 简化判断：场景规则名包含"_ordering" 且 APP 列表非空 */
        if (SnapshotList[Idx].appUsageCount > 0
            && strstr(matched[i].sceneName, "_ordering") != NULL) {
            hasAppMatch = 1;
        }
        BacktrackStrategy strategy = DecideBacktrackStrategy(&matched[i], hasAppMatch);
        int intentIdx = -1;
        int btRet = BacktrackIntentSnapshot(SnapshotList, SnapshotCount, Idx,
                                            &matched[i], strategy, &intentIdx);
        if (btRet != 0) continue;
        if (Result->candidateCount >= MAX_INTENT_CANDIDATES) break;
        IntentCandidate *cand = &Result->candidateList[Result->candidateCount++];
        cand->intentSnapshot = &SnapshotList[intentIdx];
        Snapshot evidence[MAX_EVIDENCE_PER_INTENT];
        int evidenceCount = 0;
        CollectEvidence(SnapshotList, SnapshotCount, Idx, intentIdx,
                        &matched[i], evidence, &evidenceCount);
        FillCaption(evidence, evidenceCount);
        cand->evidenceCount = evidenceCount;
        cand->evidenceList = NULL;
        if (evidenceCount > 0) {
            cand->evidenceList = (Snapshot *)malloc(sizeof(Snapshot) * evidenceCount);
            if (cand->evidenceList == NULL) {
                LOG_ERROR("malloc failed for evidenceList");
                Result->candidateCount--;
                continue;
            }
            memcpy(cand->evidenceList, evidence, sizeof(Snapshot) * evidenceCount);
        }
        cand->sceneName = matched[i].sceneName;
    }
    return INTENT_FILTER_OK;
}

/* 主入口：对快照批次按序处理，每个快照独立尝试匹配+回溯+证据收集 */
int IntentFilterProcess(Snapshot *SnapshotList, int SnapshotCount,
                        IntentResult *Result) {
    if (SnapshotList == NULL || Result == NULL || gConfig == NULL) {
        LOG_ERROR("Invalid params for IntentFilterProcess");
        return INTENT_FILTER_ERR_PARAM;
    }
    if (Result->candidateList == NULL) {
        LOG_ERROR("Result->candidateList not allocated");
        return INTENT_FILTER_ERR_PARAM;
    }
    Result->candidateCount = 0;
    int ret;
    for (int i = 0; i < SnapshotCount; i++) {
        ret = ProcessOneSnapshot(SnapshotList, SnapshotCount, i, Result);
        if (ret != INTENT_FILTER_OK) {
            LOG_ERROR("ProcessOneSnapshot failed at idx=%d", i);
            IntentResultDestroy(Result);
            return ret;
        }
    }
    LOG_INFO("IntentFilterProcess: %d candidates from %d snapshots",
             Result->candidateCount, SnapshotCount);
    return INTENT_FILTER_OK;
}

/* 释放 IntentResult 内的 evidenceList 数组（sceneName 是 SceneConfig 字符串别名，不释放） */
void IntentResultDestroy(IntentResult *Result) {
    if (Result == NULL) return;
    for (int i = 0; i < Result->candidateCount; i++) {
        free(Result->candidateList[i].evidenceList);
        Result->candidateList[i].evidenceList = NULL;
    }
    Result->candidateCount = 0;
}