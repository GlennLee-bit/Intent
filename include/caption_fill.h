#ifndef CAPTION_FILL_H
#define CAPTION_FILL_H

#include "intent_types.h"

/**
 * 将记忆表中的 caption 填充回证据快照
 * 当前实现：直接使用 latestCompanionMemoryInfo 中已有的 caption
 * 后续可扩展为查询外部记忆表
 */
void FillCaption(Snapshot *EvidenceList, int EvidenceCount);

#endif /* CAPTION_FILL_H */