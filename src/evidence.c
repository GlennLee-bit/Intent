#include "evidence.h"
#include "intent_log.h"

/* 判断快照是否与场景相关：基于 Rule 的 evidenceFields
 * 当前简化：notifications / pasteBoard / appUsage 任一非空即视为相关 */
static int IsSnapshotRelevant(Snapshot *Snapshot, SceneRule *Rule) {
    for (int i = 0; i < Rule->fieldView.evidenceFieldCount; i++) {
        FieldType field = Rule->fieldView.evidenceFields[i];
        switch (field) {
            case FIELD_NOTIFICATIONS_10MIN:
                if (Snapshot->notificationCount > 0) return 1;
                break;
            case FIELD_PASTE_BOARD_5MIN:
                if (Snapshot->pasteBoardCount > 0) return 1;
                break;
            case FIELD_APP_USAGE_30MIN:
                if (Snapshot->appUsageCount > 0) return 1;
                break;
            default:
                break;
        }
    }
    return 0;
}

/* 在 [intentIdx] 时间点附近的 [lookBackMs, lookForwardMs] 窗口内
 * 收集与场景相关的快照（排除意图快照本身） */
int CollectEvidence(Snapshot *SnapshotList, int SnapshotCount,
                    int StartIdx, int IntentIdx, SceneRule *Rule,
                    Snapshot *EvidenceList, int *EvidenceCount) {
    (void)StartIdx;
    if (SnapshotList == NULL || Rule == NULL
        || EvidenceList == NULL || EvidenceCount == NULL) {
        return INTENT_FILTER_ERR_PARAM;
    }
    if (IntentIdx < 0 || IntentIdx >= SnapshotCount) {
        return INTENT_FILTER_ERR_PARAM;
    }
    long long intentTime = SnapshotList[IntentIdx].snapshotTime;
    long long windowStart = intentTime - Rule->fieldView.lookBackMs;
    long long windowEnd = intentTime + Rule->fieldView.lookForwardMs;
    int count = 0;
    for (int i = 0; i < SnapshotCount && count < MAX_EVIDENCE_PER_INTENT; i++) {
        long long t = SnapshotList[i].snapshotTime;
        if (t < windowStart || t > windowEnd) continue;
        /* 证据列表不包含意图快照本身 */
        if (i == IntentIdx) continue;
        /* 只保留与场景相关的快照 */
        if (!IsSnapshotRelevant(&SnapshotList[i], Rule)) continue;
        EvidenceList[count++] = SnapshotList[i];
    }
    *EvidenceCount = count;
    return INTENT_FILTER_OK;
}