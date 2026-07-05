#include "caption_fill.h"
#include "intent_log.h"

/* 当前实现：caption 已在 SnapshotParseFromJson 中从 JSON 填充
 * 此函数预留为查询外部记忆表的扩展点 */
void FillCaption(Snapshot *EvidenceList, int EvidenceCount) {
    if (EvidenceList == NULL || EvidenceCount <= 0) {
        LOG_WARN("FillCaption called with empty list");
        return;
    }
    LOG_INFO("FillCaption processing %d evidence snapshots", EvidenceCount);
    /* TODO future: query external memory table by entityId */
}