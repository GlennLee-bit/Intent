#ifndef EVIDENCE_H
#define EVIDENCE_H

#include "intent_types.h"

#define MAX_EVIDENCE_PER_INTENT 64

int CollectEvidence(Snapshot *SnapshotList, int SnapshotCount,
                    int StartIdx, int IntentIdx, SceneRule *Rule,
                    Snapshot *EvidenceList, int *EvidenceCount);

#endif /* EVIDENCE_H */